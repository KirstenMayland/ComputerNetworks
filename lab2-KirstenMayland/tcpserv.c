// Kirsten Mayland
// Lab 2 Computer Networks - Exercise 2
// Spring 2024

/*
-- INSTRUCTIONS ------------------------------------------------
Implement a TCP server for the exact same protocol (version 1) as in Exercise 1.

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

#define SERV_TCP_PORT   5050
#define SERV_HOST_ADDR  "129.170.212.8"

void err_dump(char *);
void process_request(int sockfd);

int main(int argc, char	*argv[])
{
    int    sockfd, newsockfd, clilen, childpid;
    struct sockaddr_in	cli_addr, serv_addr;
    
    // Open a TCP socket (an Internet stream socket).
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        err_dump("TCP Server: Can't open stream socket");
    
    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

    // Bind our local address so that the client can send to us.
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); //inet_addr(SERV_HOST_ADDR); 
    serv_addr.sin_port        = htons(SERV_TCP_PORT);
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        err_dump("TCP Server: can't bind local address");
    
    listen(sockfd, 5);
    
    for ( ; ; ) {
        // Wait for a connection from a client process.
        // This is an example of a concurrent server.
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if (newsockfd < 0)
            err_dump("TCP server: accept error");
            
        if ( (childpid = fork()) < 0)
            err_dump("TCP server: fork error");
        
        if (childpid == 0) {  // child process
            close(sockfd);  // close original socket
            puts("In child, calling process_request");
            process_request(newsockfd);  // process the request, loops until peer closes connection
            exit(0);
        }
            
        close(newsockfd); // parent process
    }
}

void process_request(int sockfd)
{
    int     n, len;
    char    buff[MAXLINE];
    
    uint8_t  version, opcode, status;
    char statecode[2];
    char *p;

    struct response res;
    char str[MAXLINE];

    // keep reading until there's nothing to read or an error occurs
    for ( ; ; ) {
        n = read(sockfd, buff, MAXLINE);
        printf("read %d bytes\n", n);
            
        if (n == 0)
            return;  // connection terminated
        else if (n < 0){
            printf("Processing request: read error");
            return;
        }
        else if (n < 5){ //TODO: NOTE: magic integers are bad! What exactly is "5" and is it special? Make this better. 
            printf("Processing request: truncated input");
            return;
        }

        // get version from request
        p = buff;
        version = *(uint8_t *)p;
        printf("Got version: %d\n", version);
        res.version = version;

        // get opcode from request
        p += 1;
        opcode = *(uint8_t *)p;
        printf("Got opcode: %d\n", opcode);

        p += 1;
        statecode[0] = *(char *)p; // TODO: figure out how to pull out rest of data
        printf("Got statecode: %d\n", opcode);

        // TODO: get data from files

        res.status = 1;
        
        // sprintf(str, "%s", data);
        len = strlen(str);
        res.len = htons(len);

        strncpy( res.str, str, len );

        n = len + sizeof(res.version) + sizeof(res.status) + sizeof(res.len);  // len + 1 + 1 + 4
        if (write(sockfd, (void*) &res, n) != n){
            printf("process_request: write error");
            return;
        }
    }
}

void err_dump(char *msg)
{
    perror(msg);
    exit(1);
}
