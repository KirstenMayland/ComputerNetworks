// Kirsten Mayland
// Lab 2 Computer Networks - Exercise 2 Part 3
// Spring 2024

/*
-- CREDIT ------------------------------------------------
* Adapted from "tcpserv.c" which was provided in class as c code of a demo server using TCP protocol
* The demo code was adapted from W.R. Stevens' "Unix Network Programming", 1st ed. 
 */

#include  <stdio.h>
#include  <sys/types.h>
#include  <sys/socket.h>
#include  <netinet/in.h>
#include  <arpa/inet.h>
#include  <stdio.h>
#include  <strings.h> // bzero
#include  <string.h>  // bzero
#include  <stdlib.h>  // exit
#include  <unistd.h>  // close
#include  <stdint.h>  // uint8_t
#include  <ctype.h>
#include "database.h"

#define SERV_HOST_ADDR  "129.170.212.8"

// local functions
void err_dump(char *msg, struct state** values, int size);
void process_request(int sockfd, char (*keys)[MAXLINE], struct state** values, int size);
char* get_gif_filename( char* statecode, struct state** values, int size );
void send_header_v2(int sockfd, struct request2* req, struct response2 res, struct state** values, int size, int status, int reserved);
void send_gif(int sockfd, char* statecode, struct state** values, int size);
void send_string_data(int sockfd, char* statecode, int opcode, struct state** values, int index, int size);
void send_error(int sockfd, char* error_msg, struct state** values, int size);
int check_valid_query(int sockfd, struct request2 *req, struct response2 res, struct query2 query, char (*keys)[MAXLINE], struct state** values, int size);
char* query_database(char* statecode, int opcode, int index, struct state** values);

// ------------------------------main------------------------------
int main(int argc, char	*argv[])
{
    int    sockfd, newsockfd, childpid, serv_tcp_port;
    socklen_t clilen;
    struct sockaddr_in	cli_addr, serv_addr;

    // check proper input format (general)
    if( argc != 2 ){
        fprintf(stderr, "Usage: %s <port>, where <port> is an integer > 1204 that represents the port to listen on\n", argv[0]);
        exit(-1);
    } else if ( atoi(argv[1]) < 1025) {
        fprintf(stderr, "Port number to low, doesn't have permission to listen. Please enter a port > 1204\n");
        exit(-1);
    }

    serv_tcp_port = atoi(argv[1]);
    
    // Open a TCP socket (an Internet stream socket).
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "TCP Server: Can't open stream socket\n");
        exit(-1);
    } else {
        printf("TCP Server: Socket sucessfully created..\n");
    }
    
    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

    // Bind our local address so that the client can send to us.
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); //inet_addr(SERV_HOST_ADDR); 
    serv_addr.sin_port        = htons(serv_tcp_port);
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr, "TCP Server: can't bind local address\n");
        exit(-1);
    } else {
        printf("TCP Server: Successfully bound local address..\n");
    }
    
    listen(sockfd, 5);

    int size = 0;   // Current number of elements in the map 
    char keys[NUM_STATES][MAXLINE];
    struct state* values[NUM_STATES];
    create_database(TEXT_DATABASE, keys, values, &size);
    printf("TCP Server: Created database...\n");
    
    for ( ; ; ) {
        // Wait for a connection from a client process.
        // This is an example of a concurrent server.
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if (newsockfd < 0)
            err_dump("TCP Server: accept error", values, size);
            
        if ( (childpid = fork()) < 0)
            err_dump("TCP Server: fork error", values, size);
        
        if (childpid == 0) {  // child process
            close(sockfd);  // close original socket
            printf("TCP Server: In child, calling process_request...\n");
            process_request(newsockfd, keys, values, size);  // process the request, loops until peer closes connection
            exit(0);
        }
            
        close(newsockfd); // parent process
    }
    freeMap(values, size);
}

// ------------------------------process_request------------------------------
void process_request(int sockfd, char (*keys)[MAXLINE], struct state** values, int size) {
    int n;
    struct request2* req;
    struct response2 res;

    // receive header message from client -----------------
    int req_size = sizeof(*req);
    char header_buff[req_size];
    n = read(sockfd, header_buff, req_size); 
    if (n < req_size) { // 
        err_dump("TCP Server 2: process_request, header read error", values, size);
    }
    req = (struct request2 *)header_buff;

    // receive data messages from client -----------------
    struct query2 queries[req->num_queries]; // TODO: remember to free all queries
    for (int i = 0 ; i < req->num_queries; i++){
        struct query2* query = malloc(sizeof(struct query2));

        // read in query from request
        int query_size = sizeof(*query);
        char data_buff[query_size];
        n = read(sockfd, data_buff, query_size); 
        if (n < query_size) { // 
            err_dump("TCP Server 2: process_request, query read error", values, size);
        }

        // store query in arrya
        query = (struct query2 *)data_buff;
        queries[i] = *query;
    }

    // check query validities -----------------
    int database_indicies[req->num_queries];
    for (int i = 0 ; i < req->num_queries; i++){
        struct query2 query = queries[i];

        // check query validity
        int index = check_valid_query(sockfd, req, res, query, keys, values, size);   
        if (index < 0) { 
            printf("TCP Server 2: Error response to query %d sent...\n", i+1);
            return;
        }
        database_indicies[i] = index; // if valid
    }

    printf("TCP Server: All queries are valid...\n");

    // send header response back -----------------
    send_header_v2(sockfd, req, res, values, size, 1, 0);

    // send answer responses back -----------------
    for (int i = 0 ; i < req->num_queries; i++){
        struct query2 query = queries[i];

        int index = database_indicies[i];
        if (query.opcode == 5) {
            send_gif(sockfd, query.statecode, values, size);
        }
        else {   
            send_string_data(sockfd, query.statecode, query.opcode, values, index, size);
        }
        printf("TCP Server 2: Response %d sent...\n", i+1);
    }
}
// ------------------------------send_header_v2------------------------------
void send_header_v2(int sockfd, struct request2* req, struct response2 res, struct state** values, int size, int status, int reserved) {
    res.version = req->version;
    res.status = status;
    res.num_queries = req->num_queries;
    res.reserved = reserved;

    int response_size = sizeof(res);
    int n = write(sockfd, (void*) &res, response_size);
    if ( n < response_size ){
        err_dump("TCP Server 2: Error sending response", values, size);
        return;
    }
}

// ------------------------------send_string_data------------------------------
void send_string_data(int sockfd, char* statecode, int opcode, struct state** values, int index, int size) {
    int n, sz, length;
    long string_len;

    // get string and its length
    char* string = query_database(statecode, opcode, index, values);
    string_len = strlen(string);

    // send length
    sz = sizeof(uint32_t);
    length = htonl(string_len);
    n = write(sockfd, (void*) &length, sz);
    if ( n < sz ){
        err_dump("TCP Server: Error sending response", values, size);
        return;
    }

    // send data
    n = write(sockfd, (void*) string, string_len);
    if ( n < string_len ){
        err_dump("TCP Server 2: Error sending response", values, size);
        return;
    }
}

// ------------------------------send_gif------------------------------
void send_gif(int sockfd, char* statecode, struct state** values, int size) {
    int n, sz, length;
    long len;

    // first get filename and open file in rb
    char * file = get_gif_filename(statecode, values, size);
    FILE *gifp = fopen(file, "rb");
    if (gifp == NULL) {
        fprintf(stderr, "TCP Server 2: process_gif: failed to open %s", file);
        free(file);
        freeMap(values, size);
        exit(1);
    }
    // Determine size of file
    fseek(gifp, 0L, SEEK_END);
    len = ftell(gifp);
    rewind(gifp);

    // send length
    sz = sizeof(uint32_t);
    length = htonl(len);
    n = write(sockfd, &length, sz);
    if ( n < sz ){
        err_dump("TCP Server: Error sending response", values, size);
        return;
    }

    // Read and send the GIF file in chunks
    ssize_t bytes_read;
    char buffer[MAXLINE];
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), gifp)) > 0) {
        if (write(sockfd, buffer, bytes_read) < 0) {
            err_dump("Error sending GIF file", values, size);
        }
    }
    fclose(gifp);
    free(file);
}

// ------------------------------send_error------------------------------
void send_error(int sockfd, char* error_msg, struct state** values, int size) {
    int n, sz, length;
    long len;

    // get string and its length
    len = strlen(error_msg);

    // send length
    sz = sizeof(uint32_t);
    length = htonl(len);
    n = write(sockfd, (void*) &length, sz);
    if ( n < sz ){
        err_dump("TCP Server: Error sending response", values, size);
        return;
    }

    // send data
    n = write(sockfd, (void*) error_msg, len);
    if ( n != len ){
        err_dump("TCP Server 2: Error sending response", values, size);
        return;
    }
}


// ------------------------------query_database------------------------------
char* query_database(char* statecode, int opcode, int index, struct state** values) {

    if (opcode == 1) {
        return values[index]->name;
    } else if (opcode == 2) {
        return values[index]->capital;
    } else if (opcode == 3) {
        return values[index]->date;
    } else if (opcode == 4) {
        return values[index]->motto;  
    }
    return "ERROR";
}

// ------------------------------get_gif_filename------------------------------
char* get_gif_filename( char* statecode, struct state** values, int size ) {
    // create file name
    char sc[2];
    sc[0] = tolower(statecode[0]);  // convert statecode to lowercase for query purposes
    sc[1] = tolower(statecode[1]);
    char *file = malloc(strlen(FLAGS_LOC) + strlen(sc) + strlen(".gif") + 1); // +1 for the null terminator
    if (file == NULL) {
        err_dump("TCP Server 2: get_gif: memory allocation failed", values, size);
    }
    strcpy(file, FLAGS_LOC);
    strcat(file, sc);
    strcat(file, ".gif");

    return file;
}

// ------------------------------check_valid_query------------------------------
int check_valid_query(int sockfd, struct request2 *req, struct response2 res, struct query2 query, char (*keys)[MAXLINE], struct state** values, int size) {
    // check opcode
    if ( query.opcode > HIGHEST_OPCODE || query.opcode < LOWEST_OPCODE) {
        send_header_v2(sockfd, req, res, values, size, 255, 0);
        send_error(sockfd, "invalid opcode", values, size);
        return -1;
    }
    // check statecode format
    if ( ! isalpha(query.statecode[0]) || ! isalpha(query.statecode[1]) )  {
        send_header_v2(sockfd, req, res, values, size, 255, 0);
        send_error(sockfd, "invalid statecode format", values, size);
        return -1;
    }

    // check if real statecode
    char sc[2];
    sc[0] = toupper(query.statecode[0]); // convert statecode to uppercase for query purposes
    sc[1] = toupper(query.statecode[1]);
    int i = getIndex(sc, keys, size);

    if ( i < 0) {
        send_header_v2(sockfd, req, res, values, size, 255, 0);
        send_error(sockfd, "statecode does not exist", values, size);
        return -1;
    }

    return i;  // if statecode is in database, get consequent data
}

// ------------------------------err_dump------------------------------
void err_dump(char *msg, struct state** values, int size)
{
    perror(msg);
    freeMap(values, size);
    exit(1);
}