/*
 * Example of client using TCP protocol, adapted from
 *   W.R. Stevens' "Unix Network Programming", 1st ed. 
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

#define SERV_TCP_PORT   5051
#define SERV_HOST_ADDR  "127.0.0.1"  

#include "pow.h"

void worker_func( int sockfd, uint32_t num, uint8_t pow )
{
     int n, i;
     char buff[MAXLINE];

     struct request req;
     struct response *res;

     req.pow = pow;
     req.num = htonl(num);

     // write(2) is not guaranteed to write all of the request in one go,
     //  so this code is not as robust as it could be. However, triggering
     //  a partial write with such short request is unlikely.
     n = write( sockfd, (void*) &req, sizeof(req));
     if( n < sizeof(req) ){
          perror( "Error sending request" );
          exit(1);
     }

     // read(2) is not guaranteed to read the entire response at once,
     //  so this code is not as robust as it could be. However, it
     //  does try to make sure that at least the first three fields
     //  of the response are present (1+4+2 by length). The third field
     //  is the length of the string message of no more that MAXLINE chars;
     //  without it we don't know how long the response is or whether it's
     //  been completely received in a single read or not.
     n = read(sockfd, buff, MAXLINE);
     if( n < 7 ){   // NOTE: magic integers are bad! What exactly is "7" and is it special? Make this better. 
          perror("Message truncated");
          exit(1);
     }
     
     res = (struct response *)buff;

     if( res->status != RESULT_STATUS_OK ){
          printf( "Something went wrong! This is what I received:\n" );
          for (i = 0; i < n; i++) {           /* print packet */
               printf("%02X%s", (uint8_t)buff[i], (i + 1)%16 ? " " : "\n");
          }
          printf("\n");
          exit(1);
     }
     else{
          int len;
          printf( "Result: %d\n", ntohl(res->result) );
          len = ntohs(res->len);
          if( len > 0 ){
               fwrite( buff + 7, len, 1, stdout );
          }
          printf("\n");          
     }
}

int main(argc, argv)
int	argc;
char	*argv[];
{
     int sockfd;
     struct sockaddr_in	serv_addr;

     unsigned int num;
     unsigned char pow;

     if( ! argv[1] || ! argv[2] ){
       fprintf(stderr, "Usage: %s <base> <power>, where <base> and <power> are integers\n", argv[0]);
       exit(-1);
     }
     
     num = atoi(argv[1]);
     pow = atoi(argv[2]);


     /*
      * Fill in the structure "serv_addr" with the address of the
      * server that we want to connect with.
      */
     
     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family      = AF_INET;  /* "Internet", meaning IPv4 */

     // These values are formatted for immediated use with a packet's fields,
     //   hence they are represented exactly as they'd appear in a packet.
     serv_addr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR); /* string to 4-byte IPv4 address */
     serv_addr.sin_port        = htons(SERV_TCP_PORT);      /* must be network-ordered! */
     
     /*
      * Open a TCP socket (an Internet stream socket).
      */
     
     if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){  /* AF_INET is 2, SOCK_STREAM is 1 */
          perror("tcp client: can't open stream socket");
          exit(1);
     }

     /*
      * Connect to the server.
      */
     
     if (connect(sockfd, (struct sockaddr *) &serv_addr,
                 sizeof(serv_addr)) < 0){
          perror("tcp client: can't connect to server");
          exit(1);
     }

     worker_func(sockfd, num, pow);
     
     close(sockfd);
     return 0;
}

