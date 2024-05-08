/*
 * Example of server using TCP protocol.
 * Adapted from W.R. Stevens, "Unix Network Programming"
 */

#include  <stdio.h>
#include  <sys/types.h>
#include  <sys/socket.h>
#include  <netinet/in.h>
#include  <arpa/inet.h>

#include  <stdio.h>
#include  <strings.h> // bzero
#include  <stdlib.h>  // exit
#include  <unistd.h>  // close

#include <stdint.h>   // uint8_t

#include "pow.h"

#define SERV_TCP_PORT   5051
#define SERV_HOST_ADDR  "127.0.0.1"

void err_dump(char *);
void process_request(int sockfd);

int main(int argc, char	*argv[])
{
     int    sockfd, newsockfd, clilen, childpid;
     struct sockaddr_in	cli_addr, serv_addr;
     
     /*
      * Open a TCP socket (an Internet stream socket).
      */
     
     if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
          err_dump("server: can't open stream socket");
     
     
     int optval = 1;
     setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
                (const void *)&optval , sizeof(int));


     /*
      * Bind our local address so that the client can send to us.
      */
     
     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family      = AF_INET;
     serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); //inet_addr(SERV_HOST_ADDR); 
     serv_addr.sin_port        = htons(SERV_TCP_PORT);
     
     if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
          err_dump("server: can't bind local address");
     
     listen(sockfd, 5);
     
     for ( ; ; ) {
          /*
           * Wait for a connection from a client process.
           * This is an example of a concurrent server.
           */
          
          clilen = sizeof(cli_addr);
          newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,
                             &clilen);
          if (newsockfd < 0)
               err_dump("server: accept error");
               
          if ( (childpid = fork()) < 0)
               err_dump("server: fork error");
          
          if (childpid == 0) {	/* child process */
               close(sockfd);		    /* close original socket */
               puts("In child, calling process_request");
               process_request(newsockfd);  /* process the request, loops until peer closes connection */
               exit(0);
          }
               
          close(newsockfd);		/* parent process */
     }
}

void process_request(int sockfd)
{
     int     n, len, i;
     char    buff[MAXLINE];
     
     uint32_t num, result;
     uint8_t  pow;
     char *p;

     struct response res;
     char str[MAXLINE];

     for ( ; ; ) {
          n = read(sockfd, buff, MAXLINE);
          printf("read %d bytes\n", n);
               
          if (n == 0)
               return;         /* connection terminated */
          else if (n < 0){
               printf("process_request: read error");
               return;
          }
          else if (n < 5){
               printf("process_request: truncated input");
               return;
          }

          p = buff;
          pow = *(uint8_t *)p;
          p += 1;
          num = ntohl( *(uint32_t *)p );
          printf("Got num %d pow %d\n", num, pow);

          result = 1;
          for( i=0 ; i<pow; i++ ){
               result *= num;
          }
          printf("Sending back result %d\n", result);
 
          res.status = 1;
          res.result = htonl(result);
          
          sprintf(str, "%d", result);
          len = strlen(str);
          res.len = htons(len);

          strncpy( res.str, str, len );

          n = len + 1 + 4 + 2;
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
