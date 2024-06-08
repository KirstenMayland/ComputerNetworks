// Kirsten Mayland
// Lab 2 Computer Networks - Exercise 1
// Spring 2024

/*
-- CREDIT ------------------------------------------------
* Adapted from "tcpcli.c" which was provided in class as c code of a demo client using TCP protocol
* The demo code was adapted from W.R. Stevens' "Unix Network Programming", 1st ed. 
 */

#include  <stdio.h>
#include  <sys/types.h>
#include  <sys/socket.h>
#include  <netinet/in.h>
#include  <arpa/inet.h>
#include  <openssl/ossl_typ.h>
#include  <openssl/ssl.h>
#include  <openssl/err.h>
#include  <stdio.h>
#include  <string.h>  // strncpy
#include  <strings.h> // bzero
#include  <stdlib.h>  // exit
#include  <unistd.h>  // close
#include  <stdint.h>   // uint8_t
#include  <ctype.h>
#include  "client.h"
#include  "authentication.h"


#define SERV_TCP_PORT   8901 // thepond
#define SERV_HOST_ADDR  "129.170.212.8" // thepond

#define VERSION         1

// local functions
void worker_func( SSL *ssl, char statecode[2], uint8_t opcode );

// ------------------------------main------------------------------
int main( int argc, char *argv[] )
{
    int sockfd;
    struct sockaddr_in serv_addr;
    SSL_CTX *ctx;
    SSL *ssl;

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

    // configure SSL authentication
    initialize_ssl();
    ctx = create_context(1, 0);

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

    ssl = SSL_new(ctx);
    if (!ssl) {
        perror("TCP client: Unable to create SSL");
        exit(1);
    }
    SSL_set_fd(ssl, sockfd);

    if (SSL_connect(ssl) == -1) {
        perror("TCP client: Unable to connect to SSL");
        exit(1);
    } else {
        printf("TCP client: Connected to server with %s encryption...\n", SSL_get_cipher_name(ssl));
        // properly format request, send it, and receive answer
        worker_func(ssl, statecode, opcode);
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
    
    close(sockfd);
    SSL_CTX_free(ctx);
    cleanup_ssl();
    return 0;
}

// ------------------------------worker_func------------------------------
void worker_func( SSL *ssl, char statecode[2], uint8_t opcode )
{
    int n, i;
    struct request req;
    struct response *res;

    req.opcode = opcode;
    req.statecode[0] = statecode[0]; 
    req.statecode[1] = statecode[1];
    req.version = VERSION;

    // send request to server
    n = SSL_write( ssl, (void*) &req, sizeof(req));
    if( n < sizeof(req) ){
        perror( "TCP CLient: Error sending request" );
        exit(1);
    }

    // NOTE: works as is, but here's another option
    // 1. Receive header.
    // 2. Header has the size of the flag in the payload field.
    // 3. Allocate memory for flag.
    // 4. Receive flag from socket and write to file.

    // receive message from server
    int total_bytes_read = 0;
    int size_of_buff = MAXLINE;
    char *buff = (char *)malloc(size_of_buff * sizeof(char)); // Allocate memory for buffer
    if (buff == NULL) {
        perror( "TCP Client: Buffer memory allocation failed" );
        exit(1);
    }

    for ( ; ; ) {  // keep reading into the buffer and increasing the size as necessary
        n = SSL_read(ssl, buff + total_bytes_read, size_of_buff - total_bytes_read);

        if (n > 0) {
            total_bytes_read += n;
            if (total_bytes_read >= size_of_buff) {
                size_of_buff *= 2;
                // Resize buffer
                buff = (char *)realloc(buff, size_of_buff * sizeof(char)); 
                if (buff == NULL) {
                    perror( "TCP Client: Buffer memory reallocation failed" );
                    exit(1);
                }
            }
        } else if (n == 0) {
            // End of file
            break;
        }
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
        free(buff);
        exit(1);
    }
    else{ // if result status is okay, show response
        int len;       
        len = ntohl(res->len);
        if( len > 0 ){   
            // if receiving gifs
            int header_size = sizeof(*res);
            if (opcode == 5 ) {
                process_gif(statecode, len, buff + header_size);
            }
            else {
                fwrite( buff + header_size, len, 1, stdout );  // number of bytes of offset, (unit8) + (uint8) + (uint32) , i.e., 1 + 1 + 4 = 6.
            }
        }
        free(buff);
        printf("\n");          
    }
}