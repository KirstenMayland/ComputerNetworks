// Kirsten Mayland
// Lab 2 Computer Networks - Exercise 1
// Spring 2024

/*
-- INSTRUCTIONS ------------------------------------------------
Write a C program that sends properly formatted queries and displays the returned information.
For target outputs, see lab2 pdf

In this example, the address and port of the server are hard-coded in the program.
This cannot be allowed in a real-world software deliverable, but will save you some coding around C strings.

The protocol: The following opcodes are implemented:
    * 1 for the name of the state
    * 2 for its capital
    * 3 for the date when it officially became a state,
    * 4 for the state motto
    * 5 for the state flag (as a GIF)

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

#define VERSION         1
int timeout_in_seconds = 1;

// local functions
void worker_func( int sockfd, char statecode[2], uint8_t opcode, struct sockaddr_in serv_addr, socklen_t addr_len );

// ------------------------------main------------------------------
int main( int argc, char *argv[] )
{
    int sockfd;
    struct sockaddr_in serv_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    char statecode[2];
    unsigned int opcode;

    // check proper input format (general)
    if( argc != 3 || ! argv[1] || ! argv[2] || argv[1][2] || ! argv[1][1] ){
        fprintf(stderr, "Usage: %s <state code> <opcode>, where <state code> is valid two letter code (eg. pa, Wi, NH) and <opcode> is an integer from %d to %d inclusive\n", argv[0], LOWEST_OPCODE, HIGHEST_OPCODE);
        exit(-1);
    }

    opcode = atoi(argv[2]);
    statecode[0] = argv[1][0];
    statecode[1] = argv[1][1];

    // Fill in the structure "serv_addr" with the address of the server that we want to connect with.
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;  /* "Internet", meaning IPv4 */
    serv_addr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR); /* string to 4-byte IPv4 address */
    serv_addr.sin_port        = htons(SERV_UDP_PORT);      /* must be network-ordered! */
    
    // Open a UDP socket (an Internet stream socket).
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){  /* AF_INET is 2, SOCK_STREAM is 1 */
        perror("UDP client: Can't open stream socket");
        exit(1);
    }
    else {
        printf("UDP client: Socket sucessfully created..\n");
    }

    struct timeval tv;
    tv.tv_sec = timeout_in_seconds;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    // properly format request, send it, and receive answer
    worker_func(sockfd, statecode, opcode, serv_addr, addr_len);
    
    close(sockfd);
    return 0;
}

// ------------------------------worker_func------------------------------
void worker_func( int sockfd, char statecode[2], uint8_t opcode, struct sockaddr_in serv_addr, socklen_t addr_len )
{
    int n, i;
    struct request req;
    struct response *res;

    req.opcode = opcode;
    req.statecode[0] = statecode[0]; 
    req.statecode[1] = statecode[1];
    req.version = VERSION;

    // send request to server
    n = sendto(sockfd, (void*) &req, sizeof(req), 0, (struct sockaddr *)&serv_addr, addr_len);
    if( n < sizeof(req) ){
        perror("UDP CLient: Error sending request");
        exit(1);
    }

    // receive header
    char buff[MAXLINE];
    n = recvfrom(sockfd, buff, sizeof(*res), 0,(struct sockaddr *)&serv_addr, &addr_len);
    if (n < 0) {
        perror("UDP CLient: Error receiving header, try TCP");
    }
    res = (struct response *)buff;

    // recieve general data
    int size_of_buffer = ntohl(res->len);
    char* buffer = (char *)malloc(size_of_buffer); // Allocate memory for buffer
    if (buffer == NULL) {
        perror( "UDP Client: Buffer memory allocation failed" );
        exit(1);
    }
    n = recvfrom(sockfd, buffer, size_of_buffer, 0,(struct sockaddr *)&serv_addr, &addr_len);
    if (n < 0) {
        perror("UDP Client: Error receiving data, try TCP");
    }

    // process response
    // check if valid result
    if( res->status != RESULT_STATUS_OK ){
        printf( "Something went wrong! This is what I received:\n" );
        for (i = 0; i < n; i++) {           /* print packet */
            printf("%02X%s", (uint8_t)buff[i], (i + 1)%16 ? " " : "\n");
        }
        printf("\nPlease check you that you have the correct usage.\n");
        free(buffer);
        exit(1);
    }
    else{ // if result status is okay, show response
        int len;       
        len = ntohl(res->len);
        if( len > 0 ){   
            // if receiving gifs
            if (opcode == 5 ) {
                process_gif(statecode, len, buffer);
            }
            else {
                fwrite( buffer, len, 1, stdout );  // number of bytes of offset, (unit8) + (uint8) + (uint32) , i.e., 1 + 1 + 4 = 6.
            }
        }
        free(buffer);
        printf("\n");          
    }
}