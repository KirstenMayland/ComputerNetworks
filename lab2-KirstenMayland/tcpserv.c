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
#include  <string.h> // bzero
#include  <stdlib.h>  // exit
#include  <unistd.h>  // close
#include <stdint.h>   // uint8_t
#include "structs.h"

#define SERV_HOST_ADDR  "129.170.212.8"
#define NUM_STATES      50

// creating database
int size = 0;   // Current number of elements in the map 
char keys[NUM_STATES][HIGHEST_OPCODE-1];
int values[HIGHEST_OPCODE-1];
char* text_database = "data/statedb.txt";

// local functions
void err_dump(char *);
void process_request(int sockfd);
int getIndex(char key[]);
void insert(char key[], int value);
int get(char key[]);

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
        err_dump("TCP Server: Can't open stream socket");
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
        err_dump("TCP Server: can't bind local address");
    } else {
        printf("TCP Server: Successfully bound local address..\n");
    }
    
    listen(sockfd, 5);
    
    for ( ; ; ) {
        // Wait for a connection from a client process.
        // This is an example of a concurrent server.
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if (newsockfd < 0)
            err_dump("TCP Server: accept error");
            
        if ( (childpid = fork()) < 0)
            err_dump("TCP Server: fork error");
        
        if (childpid == 0) {  // child process
            close(sockfd);  // close original socket
            puts("TCP Server: In child, calling process_request");
            process_request(newsockfd);  // process the request, loops until peer closes connection
            exit(0);
        }
            
        close(newsockfd); // parent process
    }
}

// ------------------------------process_request------------------------------
void process_request(int sockfd)
{
    int     n, len;    
    uint8_t  version, opcode, status;
    char statecode[2];
    char *p;

    struct response res;
    char str[MAXLINE];

    // receive message from client -----------------
    int total_bytes_read = 0;
    int size_of_buff = MAXLINE;
    char *buff = (char *)malloc(size_of_buff * sizeof(char)); // Allocate memory for buffer
    if (buff == NULL) {
        err_dump( "TCP Server: Buffer memory allocation failed" );
    }

    // TODO: enters here, doesn't appear to exit
    for ( ; ; ) {  // keep reading into the buffer and increasing the size as necessary
        n = read(sockfd, buff + total_bytes_read, size_of_buff - total_bytes_read);

        if (n > 0) {
            total_bytes_read += n;
            if (total_bytes_read >= size_of_buff) {
                size_of_buff *= 2;
                // Resize buffer
                buff = (char *)realloc(buff, size_of_buff * sizeof(char)); 
                if (buff == NULL) {
                    err_dump( "TCP Server: Buffer memory reallocation failed" );
                }
            }
        } else if (n == 0) {  // End of request
            printf("TCP Server: Has finished reading request..\n");
            break;
        }
        else if (n < 0){
            err_dump("TCP Server: process_request, read error");
        }
    }

    // process request from client -----------------
    res.status = 1;

    // get version from request and write into response
    p = buff;
    version = *(uint8_t *)p;
    res.version = version;
    printf("Got version: %d\n", version);
    p += 1;

    // get opcode from request
    opcode = *(uint8_t *)p;
    printf("Got opcode: %d\n", opcode);
    p += 1;

    printf("Got statecode: %s\n", p);
    //str = query_database();    // TODO: get data from files

    // sprintf(str, "%s", data); // uncomment when complete above
    len = strlen(str);
    res.len = htons(len);

    strncpy( res.str, str, len );
    n = len + sizeof(res.version) + sizeof(res.status) + sizeof(res.len);  // len + 1 + 1 + 4
    if (write(sockfd, (void*) &res, n) != n){
        err_dump("TCP Server: process_request, write error");
        return;
    }
}

// ------------------------------err_dump------------------------------
void err_dump(char *msg)
{
    perror(msg);
    exit(1);
}

// ------------------------------query_database------------------------------
char* query_database(char* statecode, uint8_t opcode, struct response *res) {
    // check that opcode is valid
    if (opcode > HIGHEST_OPCODE || opcode < LOWEST_OPCODE) {
        res->status = -1;
    }
    
    // TODO:
    // use statecode as key and check if the state exists in the database
    // if not exit, if so
    // use opcode and case switch to get the corresponding char* from the state struct value or file from "/data/flags"
    // NOTE: linux is case sensitive and the database is upcase and the flags are lowercase

    return "hi";
}

// ------------------------------query_database------------------------------
void create_database() {
    FILE* fp = fopen(text_database, "r");
    if (fp == NULL) {
        fprintf(stderr, "TCP Server: query_database: failed to open %s\n", text_database);
        return;
    }


    // TODO:
    // go line by line, and split at the '|'
    //fgets(); strtok();
    // if string[0] aka the appreviation is not already a key,
    // add a key value pair of [string[0], struct state]
    // set the struct state's value to their corresponding values

    fclose(fp);
}
  
// ------------------------------getIndex------------------------------
// Function to get the index of a key in the keys array 
// Credit: https://www.geeksforgeeks.org/implementation-on-map-or-dictionary-data-structure-in-c/#
int getIndex(char key[]) 
{ 
    for (int i = 0; i < size; i++) { 
        if (strcmp(keys[i], key) == 0) { 
            return i; 
        } 
    } 
    return -1; // Key not found 
} 

// ------------------------------insert------------------------------  
// Function to insert a key-value pair into the map 
// Credit: https://www.geeksforgeeks.org/implementation-on-map-or-dictionary-data-structure-in-c/#
void insert(char key[], int value) 
{ 
    int index = getIndex(key); 
    if (index == -1) { // Key not found 
        strcpy(keys[size], key); 
        values[size] = value; 
        size++; 
    } 
    else { // Key found 
        values[index] = value; 
    } 
} 
  
// ------------------------------get------------------------------  
// Function to get the value of a key in the map 
// Credit: https://www.geeksforgeeks.org/implementation-on-map-or-dictionary-data-structure-in-c/#
int get(char key[]) 
{ 
    int index = getIndex(key); 
    if (index == -1) { // Key not found 
        return -1; 
    } 
    else { // Key found 
        return values[index]; 
    } 
} 