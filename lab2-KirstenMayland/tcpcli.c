// Kirsten Mayland
// Lab 2 Computer Networks - Exercise 1
// Spring 2024

/*
-- INSTRUCTIONS ------------------------------------------------
There is a server running at packetbender.com on TCP port 5050.
When queried according to the protocol described below, it returns information about a US state by its US postal two-letter code.
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
#include  <netinet/in.h>
#include  <arpa/inet.h>
#include  <stdio.h>
#include  <string.h>  // strncpy
#include  <strings.h> // bzero
#include  <stdlib.h>  // exit
#include  <unistd.h>  // close
#include <stdint.h>   // uint8_t
#include <ctype.h>
#include "structs.h"

#define SERV_TCP_PORT   5050
#define SERV_HOST_ADDR  "71.19.146.5"  

int highest_opcode = 5;
int lowest_opcode = 1;
int version = 1;

// local functions
void worker_func( int sockfd, char statecode[2], uint8_t opcode );
void create_local_gif(char statecode[2]);

// ------------------------------main------------------------------
int main( int argc, char *argv[] )
{
    int sockfd;
    struct sockaddr_in serv_addr;

    char statecode[2];
    unsigned int opcode;

    // check proper input format (general)
    if( argc != 3 || ! argv[1] || ! argv[2] || argv[1][2] || ! argv[1][1]){
        fprintf(stderr, "Usage: %s <state code> <opcode>, where <state code> is valid two letter code (eg. pa, Wi, NH) and <opcode> is an integer from %d to %d inclusive\n", argv[0], lowest_opcode, highest_opcode);
        exit(-1);
    }

    opcode = atoi(argv[2]);
    statecode[0] = argv[1][0];
    statecode[1] = argv[1][1];

    // Fill in the structure "serv_addr" with the address of the server that we want to connect with.
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;  /* "Internet", meaning IPv4 */
    serv_addr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR); /* string to 4-byte IPv4 address */
    serv_addr.sin_port        = htons(SERV_TCP_PORT);      /* must be network-ordered! */
    
    // Open a TCP socket (an Internet stream socket).
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){  /* AF_INET is 2, SOCK_STREAM is 1 */
        perror("TCP client: Can't open stream socket");
        exit(1);
    }
    else {
        printf("TCP client: Socket sucessfully created..\n");
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        perror("TCP client: Can't connect to server");
        exit(1);
    }
    else {
        printf("TCP client: Connected to server..\n");
    }

    // properly format request, send it, and receive answer
    worker_func(sockfd, statecode, opcode);
    
    close(sockfd);
    return 0;
}

// ------------------------------worker_func------------------------------
void worker_func( int sockfd, char statecode[2], uint8_t opcode )
{
    int n, i;
    char buff[MAXLINE];

    struct request req;
    struct response *res;

    req.opcode = opcode;
    req.statecode[0] = statecode[0]; 
    req.statecode[1] = statecode[1];
    req.version = version;

    // send request to client
    n = write( sockfd, (void*) &req, sizeof(req));
    if( n < sizeof(req) ){
        perror( "Error sending request" );
        exit(1);
    }

    // receive response
    n = read(sockfd, buff, MAXLINE);
    if( n < 7 ){   //TODO: NOTE: magic integers are bad! What exactly is "7" and is it special? Make this better. 
        perror("Message truncated");
        exit(1);
    }
    
    res = (struct response *)buff;

    // process response
    // check if valid result
    if( res->status != RESULT_STATUS_OK ){
        printf( "Something went wrong! This is what I received:\n" );
        for (i = 0; i < n; i++) {           /* print packet */
            printf("%02X%s", (uint8_t)buff[i], (i + 1)%16 ? " " : "\n");
        }
        printf("\nPlease check you that you have the correct usage.\n");
        printf("Usage: ./tcpcli <state code> <opcode>, where <state code> is valid two letter code (eg. pa, Wi, NH) and <opcode> is an integer from %d to %d inclusive\n", lowest_opcode, highest_opcode);
        exit(1);
    }
    else{ // if result status is okay, show response
        int len;       
        len = ntohl(res->len);
        if( len > 0 ){   
            // if receiving gifs
            if (opcode == 5 ) { // TODO: Figure out how to represent gif as not the bytes
                create_local_gif(statecode);
            }
            else {
                fwrite( buff + sizeof(res->version) + sizeof(res->status) + sizeof(res->len), len, 1, stdout );  // number of bytes of offset, (unit8) + (uint8) + (uint32) , i.e., 1 + 1 + 4 = 6.
            }
        }
        printf("\n");          
    }
}

// ------------------------------create_local_gif------------------------------
void create_local_gif(char statecode[2]) {

    // create file naem
    char *str = malloc(strlen(statecode) + strlen(".gif") + 1); // +1 for the null terminator
    if (str == NULL) {
        fprintf(stderr, "create_local_gif: memory allocation failed\n");
        return;
    }
    strcpy(str, statecode);
    strcat(str, ".gif");


    printf("concat: %s\n", str);

    fopen(str, "w");

    //fwrite();

    //fclose():

    free(str); // Don't forget to free dynamically allocated memory
}