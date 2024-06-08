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
#include  <sys/time.h>
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

#define SERV_UDP_PORT   8901 // thepond
#define SERV_HOST_ADDR  "129.170.212.8" // thepond

#define VERSION         1
int timeout_in_seconds = 1;

// local functions
void worker_func( SSL *ssl, char statecode[2], uint8_t opcode, struct sockaddr_in serv_addr, socklen_t addr_len );

// ------------------------------main------------------------------
int main( int argc, char *argv[] )
{
    int sockfd;
    struct sockaddr_in serv_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    SSL_CTX *ctx;
    SSL *ssl;
    BIO *bio;

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

    // initialize SSL authentication
    initialize_ssl();
    ctx = create_context(1, 1);
    SSL_CTX_set_info_callback(ctx, info_callback);
    
    // Open a UDP socket (an Internet stream socket).
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){  /* AF_INET is 2, SOCK_STREAM is 1 */
        perror("UDP Client: Can't open stream socket");
        exit(1);
    }
    else {
        printf("UDP Client: Socket sucessfully created..\n");
    }

    // Fill in the structure "serv_addr" with the address of the server that we want to connect with.
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;  /* "Internet", meaning IPv4 */
    serv_addr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR); /* string to 4-byte IPv4 address */
    serv_addr.sin_port        = htons(SERV_UDP_PORT);      /* must be network-ordered! */

    // set recvfrom timeouts
    struct timeval tv;
    tv.tv_sec = timeout_in_seconds;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    // configure SSL authentication
    ssl = SSL_new(ctx);
    bio = BIO_new_dgram(sockfd, BIO_CLOSE);
    BIO_ctrl(bio, BIO_CTRL_DGRAM_SET_CONNECTED, 0, &serv_addr);
    SSL_set_bio(ssl, bio, bio);

    if (SSL_connect(ssl) <= 0) {  // TODO: error
        perror("UDP Client: Unable to connect to SSL");
        ERR_print_errors_fp(stderr);
        exit(1);
    } else {
        printf("UDP Client: Connected to server with %s encryption...\n", SSL_get_cipher_name(ssl));
        // properly format request, send it, and receive answer
        worker_func(ssl, statecode, opcode, serv_addr, addr_len);

        SSL_shutdown(ssl);
        SSL_free(ssl);
    }

    close(sockfd);
    SSL_CTX_free(ctx);
    cleanup_ssl();
    return 0;
}

// ------------------------------worker_func------------------------------
void worker_func( SSL *ssl, char statecode[2], uint8_t opcode, struct sockaddr_in serv_addr, socklen_t addr_len )
{
    int n, i;
    struct request req;
    struct response *res;

    req.opcode = opcode;
    req.statecode[0] = statecode[0]; 
    req.statecode[1] = statecode[1];
    req.version = VERSION;

    // send request to server
    // n = sendto(sockfd, (void*) &req, sizeof(req), 0, (struct sockaddr *)&serv_addr, addr_len);
    n = SSL_write(ssl, (void*) &req, sizeof(req));
    if( n < sizeof(req) ){
        perror("UDP Client: Error sending request");
        exit(1);
    }

    // receive header
    char buff[MAXLINE];
    // n = recvfrom(sockfd, buff, sizeof(*res), 0, (struct sockaddr *)&serv_addr, &addr_len);
    n = SSL_read(ssl, buff, sizeof(*res));
    if (n < 0) {
        perror("UDP Client: Error receiving header, try TCP");
    }
    res = (struct response *)buff;

    // recieve general data
    int size_of_buffer = ntohl(res->len);
    char* buffer = (char *)malloc(size_of_buffer); // Allocate memory for buffer
    if (buffer == NULL) {
        perror( "UDP Client: Buffer memory allocation failed" );
        exit(1);
    }
    //n = recvfrom(sockfd, buffer, size_of_buffer, 0,(struct sockaddr *)&serv_addr, &addr_len);
    n = SSL_read(ssl, buffer, size_of_buffer);
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