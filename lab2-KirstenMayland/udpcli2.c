// Kirsten Mayland
// Lab 2 Computer Networks - Exercise 2 Part 3
// Spring 2024

/*
-- CREDIT ------------------------------------------------
* Adapted from "tcpcli.c" which was provided in class as c code of a demo client using TCP protocol
* The demo code was adapted from W.R. Stevens' "Unix Network Programming", 1st ed. 
 */

#include  <stdio.h>
#include  <sys/types.h>
#include  <sys/socket.h>
#include  <sys/time.h>
#include  <netinet/in.h>
#include  <arpa/inet.h>
#include  <stdio.h>
#include  <string.h>  // strncpy
#include  <strings.h> // bzero
#include  <stdlib.h>  // exit
#include  <unistd.h>  // close
#include  <stdint.h>   // uint8_t
#include  <ctype.h>
#include  "client.h"

#define SERV_UDP_PORT   8901 // thepond
#define SERV_HOST_ADDR  "129.170.212.8" // thepond

int version = 2;
int timeout_in_seconds = 1;

// local functions
void worker_func( int sockfd, int num_queries, struct query2* queries, struct sockaddr_in serv_addr, socklen_t addr_len );

// ------------------------------main------------------------------
int main( int argc, char *argv[] )
{
    int sockfd;
    struct sockaddr_in serv_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    // check proper input format (general)
    if( (argc % 2) != 1 ){
        fprintf(stderr, "Usage: %s (<state code> <opcode>)*n, where <state code> is valid two letter code (eg. pa, Wi, NH) and <opcode> is an integer from %d to %d inclusive\n", argv[0], LOWEST_OPCODE, HIGHEST_OPCODE);
        exit(-1);
    }
    // translate input to queries
    int num_queries = (argc-1)/2;
    struct query2 queries[num_queries]; 
    for (int i = 0 ; i < num_queries; i++){
        int n = (i*2)+1;

        // check opcode
        if ( atoi(argv[n+1]) > HIGHEST_OPCODE || atoi(argv[n+1]) < LOWEST_OPCODE) {
            perror("TCP Client 2: Query has invalid opcode in it");
            // TODO:free queries so far
            exit(1);    
        }

        // check statecode format
        if ( strlen(argv[n]) != 2 || ! isalpha(argv[n][0]) || ! isalpha(argv[n][0]) )  {
            perror("TCP Client 2: Query has invalid statecode in it");
            // TODO:free queries so far
            exit(1); 
        }

        // malloc new query2
        struct query2* query = malloc(sizeof(struct query2)); // Allocate memory //TODO: remember to free
        
        // set opcode and statecode
        query->opcode = atoi(argv[n+1]);
        strcpy(query->statecode, argv[n]);

        // add to query2 array
        queries[i] = *query;
    }

    // Fill in the structure "serv_addr" with the address of the server that we want to connect with.
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;  /* "Internet", meaning IPv4 */
    serv_addr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR); /* string to 4-byte IPv4 address */
    serv_addr.sin_port        = htons(SERV_UDP_PORT);      /* must be network-ordered! */
    
    // Open a UDP socket (an Internet stream socket).
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){  /* AF_INET is 2, SOCK_STREAM is 1 */
        perror("UDP Client 2: Can't open stream socket");
        exit(1);
    }
    else {
        printf("UDP Client 2: Socket sucessfully created..\n");
    }

    struct timeval tv;
    tv.tv_sec = timeout_in_seconds;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    // properly format request, send it, and receive answer
    worker_func(sockfd, num_queries, queries, serv_addr, addr_len);
    
    close(sockfd);
    return 0;
}

// ------------------------------worker_func------------------------------
void worker_func( int sockfd, int num_queries, struct query2* queries, struct sockaddr_in serv_addr, socklen_t addr_len )
{
    int n, i;
    struct request2 req;
    struct response2 *res;

    req.version = version;
    req.num_queries = num_queries;

    // send request to server -----------
    // header
    int req_size = sizeof(req);
    n = sendto(sockfd, (void*) &req, req_size, 0, (struct sockaddr *)&serv_addr, addr_len);
    if( n < req_size ){
        perror("UDP Client 2: Error sending request header");
        exit(1);
    }
    // data
        for (i = 0 ; i < num_queries; i++){
        struct query2 query = queries[i];
        int query_size = sizeof(query);

        n = sendto(sockfd, (void*) &query, query_size, 0, (struct sockaddr *)&serv_addr, addr_len);
        if( n < query_size ){
            perror( "UDP Client 2: Error sending request data" );
            exit(1);
        }
    }

    printf("UDP Client 2: Sent %d query request(s)..\n", num_queries);

    // receive header message from server -----------
    int res_size = sizeof(*res);
    char header_buff[res_size];
    n = recvfrom(sockfd, header_buff, res_size, 0,(struct sockaddr *)&serv_addr, &addr_len);
    if (n < res_size) {
        perror( "UDP Client 2: header message truncated" );
        exit(1);
    }
    res = (struct response2 *)header_buff;

    // check if response status is good
    if( res->status != RESULT_STATUS_OK ){
        printf( "Something went wrong! This is what I received:\n" );
        char buff[MAXLINE];
        n = recvfrom(sockfd, buff, MAXLINE, 0,(struct sockaddr *)&serv_addr, &addr_len);
        if (n < 0) {
            perror( "UDP Client 2: header message truncated" );
            exit(1);
        }
        for (i = 0; i < n; i++) {           /* print packet */
            printf("%02X%s", (uint8_t) buff[i], (i + 1)%16 ? " " : "\n");
        }
        printf("\nPlease check you that you have the correct usage.\n");
        exit(1);
    }
    // receive data message from server -----------
    else {
        for (i = 0 ; i < num_queries; i++){
            printf("UDP Client 2: Processing query %d...\n", i+1);
            // read in first 4 bytes to get len length
            int sz = sizeof(uint32_t);
            long length;
            n = recvfrom(sockfd, &length, sz, 0, (struct sockaddr *)&serv_addr, &addr_len);
            if (n < sz) {
                perror( "UDP Client 2: header message truncated" );
                exit(1);
            }

            // read in next len bytes
            int len = ntohl(length); 
            char data[len];
            int bytes_read = 0;
            for ( ; ; ) {
                n = recvfrom(sockfd, data + bytes_read, len - bytes_read, 0,(struct sockaddr *)&serv_addr, &addr_len);
                bytes_read += n;
                if (n < 0) {
                    perror( "UDP Client 2: read data error" );
                    exit(1);
                }
                else if (n == 0) { // end of data
                    break;
                }
                else if (bytes_read == len) { // end of data
                    break;
                }
            }

            // process resulting data
            if( len > 0 ){   
                if (queries[i].opcode == 5 ) {
                    if (len > MAXLINE) {  // if sent as normal
                        process_gif(queries[i].statecode, len, data);
                    } else { // if unable to be sent in single UDP packet
                        fwrite(data, len, 1, stdout);
                    }
                }
                else {
                    fwrite(data, len, 1, stdout);  // number of bytes of offset, (unit8) + (uint8) + (uint32) , i.e., 1 + 1 + 4 = 6.
                }
            }
            printf("\n");   
        }
    }
}