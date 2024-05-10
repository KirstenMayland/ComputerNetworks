// Kirsten Mayland
// Lab 2 Computer Networks - Exercise 2
// Spring 2024

/*
-- INSTRUCTIONS ------------------------------------------------
Implement a TCP server for the exact same protocol (version 1) as in Exercise 1.
For ease of testing, your server should take one argument: the port to listen on.

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
void send_gif(int sockfd, struct request* req, struct response res, struct state** values, int size);
void send_string_data(int sockfd, struct request* req, struct response res, struct state** values, int i, int size);
void send_error(int sockfd, struct request* req, struct response res, char* error_msg, struct state** values, int size);
int check_valid_query(int sockfd, struct request* req, struct response res, char (*keys)[MAXLINE], struct state** values, int size);
char* query_database(char* statecode, int opcode, struct response *res, int i, struct state** values);

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
    //printMap();
    
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
    struct response res;
    struct request* req;
    char buff[MAXLINE];

    // receive message from client -----------------
    n = read(sockfd, buff, MAXLINE); 
    if (n < 0) { // 
        err_dump("TCP Server: process_request, read error", values, size);
    }
    else if (n < sizeof(*req)) {
        err_dump("TCP Server: process_request, request truncated", values, size);
    }
    req = (struct request *)buff;


    // send response back -----------------
    res.version = req->version;
    res.status = 1;

    int i = check_valid_query(sockfd, req, res, keys, values, size);
    if ( i != -1 ) {
        if (req->opcode == 5) {
            send_gif(sockfd, req, res, values, size);
        }
        else {   
            send_string_data(sockfd, req, res, values, i, size);
        }
        printf("TCP Server: Response sent...\n");
    }
    else {
        printf("TCP Server: Error response sent...\n");
    }
}

// ------------------------------send_string_data------------------------------
void send_string_data(int sockfd, struct request* req, struct response res, struct state** values, int i, int size) {
    int n, len, sz;

    // get string and its length
    char* string = query_database(req->statecode, req->opcode, &res, i, values);
    len = strlen(string);
    res.len = htonl(len);

    // send header
    sz = sizeof(res.version) + sizeof(res.status) + sizeof(res.len);  //  1 + 1 + 4
    n = write(sockfd, (void*) &res, sz);
    if ( n != sz ){
        err_dump("TCP Server: Error sending response", values, size);
        return;
    }
    // send data
    n = write(sockfd, (void*) string, len);
    if ( n != len ){
        err_dump("TCP Server: Error sending response", values, size);
        return;
    }
}

// ------------------------------send_gif------------------------------
void send_gif(int sockfd, struct request* req, struct response res, struct state** values, int size) {
    int n, sz;
    long length;

    // first get filename and open file in rb
    char * file = get_gif_filename(req->statecode, values, size);
    FILE *gifp = fopen(file, "rb");
    if (gifp == NULL) {
        res.status = -1;
        fprintf(stderr, "TCP Server: process_gif: failed to open %s", file);
        free(file);
        freeMap(values, size);
        exit(1);
    }
    // then determine size of file
    fseek(gifp, 0L, SEEK_END);
    length = ftell(gifp);
    rewind(gifp);

    // add length to header and send header
    res.len = htonl(length);
    sz = sizeof(res.version) + sizeof(res.status) + sizeof(res.len);  //  1 + 1 + 4
    n = write(sockfd, (void*) &res, sz);
    if ( n != sz ){
        err_dump("TCP Server: Error sending response", values, size);
        return;
    }

    // Read and send the GIF file in chunks
    ssize_t bytes_read;
    char buffer[1024];
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), gifp)) > 0) {
        if (send(sockfd, buffer, bytes_read, 0) < 0) {
            err_dump("Error sending GIF file", values, size);
        }
    }
    fclose(gifp);
    free(file);
}

// ------------------------------send_error------------------------------
void send_error(int sockfd, struct request* req, struct response res, char* error_msg, struct state** values, int size) {
    int n, len, sz;

    res.status = 255;

    // get string and its length
    len = strlen(error_msg);
    res.len = htonl(len);

    // send header
    sz = sizeof(res.version) + sizeof(res.status) + sizeof(res.len);  //  1 + 1 + 4
    n = write(sockfd, (void*) &res, sz);
    if ( n != sz ){
        err_dump("TCP Server: Error sending response", values, size);
        return;
    }
    // send data
    n = write(sockfd, (void*) error_msg, len);
    if ( n != len ){
        err_dump("TCP Server: Error sending response", values, size);
        return;
    }
}


// ------------------------------query_database------------------------------
char* query_database(char* statecode, int opcode, struct response *res, int i, struct state** values) {

    if (opcode == 1) {
        return values[i]->name;
    } else if (opcode == 2) {
        return values[i]->capital;
    } else if (opcode == 3) {
        return values[i]->date;
    } else if (opcode == 4) {
        return values[i]->motto;  
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
        err_dump("TCP Server: get_gif: memory allocation failed", values, size);
    }
    strcpy(file, FLAGS_LOC);
    strcat(file, sc);
    strcat(file, ".gif");

    return file;
}

// ------------------------------check_valid_query------------------------------
int check_valid_query(int sockfd, struct request* req, struct response res, char (*keys)[MAXLINE], struct state** values, int size) {
    printf("TCP Server: Checking query validity...\n");
    // check opcode
    if ( req->opcode > HIGHEST_OPCODE || req->opcode < LOWEST_OPCODE) {
        send_error(sockfd, req, res, "invalid opcode", values, size);
        return -1;
    }
    // check statecode format
    if ( ! isalpha(req->statecode[0]) || ! isalpha(req->statecode[1]) )  {
        send_error(sockfd, req, res, "invalid statecode format", values, size);
        return -1;
    }

    // check if real statecode
    char sc[2];
    sc[0] = toupper(req->statecode[0]); // convert statecode to uppercase for query purposes
    sc[1] = toupper(req->statecode[1]);
    int i = getIndex(sc, keys, size);

    if ( i < 0) {
        send_error(sockfd, req, res, "statecode does not exist", values, size);
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