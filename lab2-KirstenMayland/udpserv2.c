// Kirsten Mayland
// Lab 2 Computer Networks - Exercise 2
// Spring 2024

/*
-- INSTRUCTIONS ------------------------------------------------
Implement a UDP server for the same protocol (version 1).
Listen for requests on a UDP port, and return the answer over UDP if the answer fits inside a single UDP packet.
If the answer would not fit, return a status code of 254, and the error string of “Response too large for UDP; try UDP”.

-- CREDIT ------------------------------------------------
* Adapted from my "UDPserv.c" which was an adaption of the "UDPserv.c" which was provided in class as c code of a demo server using UDP protocol
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
void process_request(int sockfd, char (*keys)[MAXLINE], struct state** values, int size, struct sockaddr_in	cli_addr, socklen_t addr_len);
char* get_gif_filename( char* statecode, struct state** values, int size );
void send_header_v2(int sockfd, struct request2* req, struct response2 res, struct state** values, int size, int status, int reserved, const struct sockaddr *cli_addr, socklen_t addr_len);
void send_gif(int sockfd, char* statecode, struct request2* req, struct response2 res, struct state** values, int size, struct sockaddr_in cli_addr, socklen_t addr_len);
void send_string_data(int sockfd, char* statecode, int opcode, struct state** values, int index, int size, struct sockaddr_in cli_addr, socklen_t addr_len);
void send_error(int sockfd, char* error_msg, struct state** values, int size, struct sockaddr_in cli_addr, socklen_t addr_len);
int check_valid_query(int sockfd, struct request2* req, struct response2 res, struct query2 query, char (*keys)[MAXLINE], struct state** values, int size, struct sockaddr_in cli_addr, socklen_t addr_len);

// ------------------------------main------------------------------
int main(int argc, char	*argv[])
{
    int    sockfd, serv_udp_port;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    struct sockaddr_in	cli_addr, serv_addr;

    // check proper input format (general)
    if( argc != 2 ){
        fprintf(stderr, "Usage: %s <port>, where <port> is an integer > 1204 that represents the port to listen on\n", argv[0]);
        exit(-1);
    } else if ( atoi(argv[1]) < 1025) {
        fprintf(stderr, "Port number to low, doesn't have permission to listen. Please enter a port > 1204\n");
        exit(-1);
    }

    serv_udp_port = atoi(argv[1]);
    
    // Open a UDP socket (an Internet stream socket).
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        fprintf(stderr, "UDP Server 2: Can't open stream socket\n");
        exit(-1);
    } else {
        printf("UDP Server 2: Socket sucessfully created..\n");
    }
    
    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

    // Bind our local address so that the client can send to us.
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); //inet_addr(SERV_HOST_ADDR); 
    serv_addr.sin_port        = htons(serv_udp_port);
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr, "UDP Server 2: can't bind local address\n");
        exit(-1);
    } else {
        printf("UDP Server 2: Successfully bound local address..\n");
    }
    
    int size = 0;   // Current number of elements in the map 
    char keys[NUM_STATES][MAXLINE];
    struct state* values[NUM_STATES];
    create_database(TEXT_DATABASE, keys, values, &size);
    printf("UDP Server 2: Created database...\n");
    
    printf("UDP Server 2: running on port %d...\n", serv_udp_port);
    for ( ; ; ) {
        process_request(sockfd, keys, values, size, cli_addr, addr_len);
    }
    close(sockfd);
    freeMap(values, size);
}

// ------------------------------process_request------------------------------
void process_request(int sockfd, char (*keys)[MAXLINE], struct state** values, int size, struct sockaddr_in	cli_addr, socklen_t addr_len) {
    int n;
    struct request2* req;
    struct response2 res;

    // receive header message from client -----------------
    int req_size = sizeof(*req);
    char header_buff[req_size];
    n = recvfrom(sockfd, header_buff, req_size, 0, (struct sockaddr *)&cli_addr, &addr_len);
    if (n < req_size) { // 
        err_dump("UDP Server 2: process_request, header read error", values, size);
    }
    req = (struct request2 *)header_buff;
    printf("receieved header: num_queries = %d, version = %d\n", req->num_queries, req->version);

    // receive data messages from client -----------------
    struct query2 queries[req->num_queries]; // TODO: remember to free all queries
    for (int i = 0 ; i < req->num_queries; i++){
        struct query2* query = malloc(sizeof(struct query2));

        // read in query from request
        int query_size = sizeof(*query);
        char data_buff[query_size];
        n = recvfrom(sockfd, data_buff, query_size, 0, (struct sockaddr *)&cli_addr, &addr_len);
        if (n < query_size) { // 
            err_dump("UDP Server 2: process_request, query read error", values, size);
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
        int index = check_valid_query(sockfd, req, res, query, keys, values, size, cli_addr, addr_len);   
        if (index < 0) { 
            printf("UDP Server 2: Error response to query %d sent...\n", i+1);
            return;
        }
        database_indicies[i] = index; // if valid
    }

    printf("UDP Server 2: All queries are valid...\n");

    // send header response back -----------------
    send_header_v2(sockfd, req, res, values, size, 1, 0, (struct sockaddr *)&cli_addr, addr_len);

    // send answer responses back -----------------
    for (int i = 0 ; i < req->num_queries; i++){
        struct query2 query = queries[i];

        int index = database_indicies[i];
        if (query.opcode == 5) {
            send_gif(sockfd, query.statecode, req, res, values, size, cli_addr, addr_len);
        }
        else {   
            send_string_data(sockfd, query.statecode, query.opcode, values, index, size, cli_addr, addr_len);
        }
        printf("UDP Server 2: Response %d sent...\n", i+1);
    }
}
// ------------------------------send_header_v2------------------------------
void send_header_v2(int sockfd, struct request2* req, struct response2 res, struct state** values, int size, int status, int reserved, const struct sockaddr *cli_addr, socklen_t addr_len) {
    int response_size, n;
    res.version = req->version;
    res.status = status;
    res.num_queries = req->num_queries;
    res.reserved = reserved;

    response_size = sizeof(res);
    n = sendto(sockfd, (void*) &res, response_size, 0, cli_addr, addr_len);
    if ( n < response_size ){
        err_dump("UDP Server 2: Error sending response", values, size);
        return;
    }
}

// ------------------------------send_string_data------------------------------
void send_string_data(int sockfd, char* statecode, int opcode, struct state** values, int index, int size, struct sockaddr_in cli_addr, socklen_t addr_len) {
    int n, sz, length;
    long string_len;

    // get string and its length
    char* string = query_database(statecode, opcode, index, values);
    string_len = strlen(string);

    if (string_len > MAXLINE) { // if can't fit inside a single UDP packet
        send_error(sockfd, "packet to big, try TCP", values, size, cli_addr, addr_len);
    }
    else {
        // send header
        sz = sizeof(uint32_t);
        length = htonl(string_len);
        n = sendto(sockfd, (void*) &length, sz, 0, (const struct sockaddr *)&cli_addr, addr_len);
        if ( n < sz ){
            err_dump("UDP Server 2: Error sending response", values, size);
            return;
        }

        // send data
        n = sendto(sockfd, (void*) string, string_len, 0, (const struct sockaddr *)&cli_addr, addr_len);
        if ( n != string_len ){
            err_dump("UDP Server 2: Error sending response", values, size);
            return;
        }
    }
}

// ------------------------------send_gif------------------------------
void send_gif(int sockfd, char* statecode, struct request2* req, struct response2 res, struct state** values, int size, struct sockaddr_in cli_addr, socklen_t addr_len) {
    int n, sz, length;
    long len;

    // first get filename and open file in rb
    char * file = get_gif_filename(statecode, values, size);
    FILE *gifp = fopen(file, "rb");
    if (gifp == NULL) {
        res.status = -1;
        fprintf(stderr, "UDP Server 2: process_gif: failed to open %s", file);
        free(file);
        freeMap(values, size);
        exit(1);
    }
    // then determine size of file
    fseek(gifp, 0L, SEEK_END);
    len = ftell(gifp);
    rewind(gifp);

    if (len > MAXLINE) { // if can't fit inside a single UDP packet
        send_error(sockfd, "packet to big, try TCP", values, size, cli_addr, addr_len);
    }
    else {
        // send length
        sz = sizeof(uint32_t);
        length = htonl(len);
        n = write(sockfd, &length, sz);
        if ( n < sz ){
            err_dump("TCP Server 2: Error sending response", values, size);
            return;
        }
        
        // Read and send the GIF file in chunks
        ssize_t bytes_read;
        char* buffer = (char *)malloc(length); // Allocate memory for buffer
        if (buffer == NULL) {
            perror( "UDP Client: Buffer memory allocation failed" );
            exit(1);
        }

        bytes_read = fread(buffer, 1, length, gifp);
        n = sendto(sockfd, (void*) buffer, bytes_read, 0, (const struct sockaddr *)&cli_addr, addr_len);
        if ( n < bytes_read) {
            err_dump("Error sending GIF file", values, size);
        }
    }

    fclose(gifp);
    free(file);
}

// ------------------------------send_error------------------------------
void send_error(int sockfd, char* error_msg, struct state** values, int size, struct sockaddr_in cli_addr, socklen_t addr_len) {
    int n, sz, length;
    long len;

    // get string and its length
    len = strlen(error_msg);

    // send length
    sz = sizeof(uint32_t);
    length = htonl(len);
    n = sendto(sockfd, (void*) &length, sz, 0, (const struct sockaddr *)&cli_addr, addr_len);
    if ( n < sz ){
        err_dump("UDP Server 2: Error sending response", values, size);
        return;
    }

    // send data
    n = sendto(sockfd, (void*) error_msg, len, 0, (const struct sockaddr *)&cli_addr, addr_len);
    if ( n != len ){
        err_dump("UDP Server 2: Error sending response", values, size);
        return;
    }
}

// ------------------------------check_valid_query------------------------------
int check_valid_query(int sockfd, struct request2* req, struct response2 res, struct query2 query, char (*keys)[MAXLINE], struct state** values, int size, struct sockaddr_in cli_addr, socklen_t addr_len) {
    printf("UDP Server 2: Checking query validity...\n");
    // check opcode
    if ( query.opcode > HIGHEST_OPCODE || query.opcode < LOWEST_OPCODE) {
        send_header_v2(sockfd, req, res, values, size, 255, 0, (const struct sockaddr *)&cli_addr, addr_len);
        send_error(sockfd, "invalid opcode", values, size, cli_addr, addr_len);
        return -1;
    }
    // check statecode format
    if ( ! isalpha(query.statecode[0]) || ! isalpha(query.statecode[1]) )  {
        send_header_v2(sockfd, req, res, values, size, 255, 0, (const struct sockaddr *)&cli_addr, addr_len);
        send_error(sockfd, "invalid statecode format", values, size, cli_addr, addr_len);
        return -1;
    }

    // check if real statecode
    char sc[2];
    sc[0] = toupper(query.statecode[0]); // convert statecode to uppercase for query purposes
    sc[1] = toupper(query.statecode[1]);
    int i = getIndex(sc, keys, size);

    if ( i < 0) {
        send_header_v2(sockfd, req, res, values, size, 255, 0, (const struct sockaddr *)&cli_addr, addr_len);
        send_error(sockfd, "statecode does not exist", values, size, cli_addr, addr_len);
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