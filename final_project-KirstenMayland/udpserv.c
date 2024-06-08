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
#include  <openssl/ossl_typ.h>
#include  <openssl/ssl.h>
#include  <openssl/err.h>
#include  <stdio.h>
#include  <strings.h> // bzero
#include  <string.h>  // bzero
#include  <stdlib.h>  // exit
#include  <unistd.h>  // close
#include  <stdint.h>  // uint8_t
#include  <ctype.h>
#include "database.h"
#include  "authentication.h"

#define SERV_HOST_ADDR  "129.170.212.8"

// local functions
void err_dump(char *msg, struct state** values, int size);
void process_request(int sockfd, char (*keys)[MAXLINE], struct state** values, int size, struct sockaddr_in	cli_addr, socklen_t addr_len, SSL_CTX *ctx);
char* get_gif_filename( char* statecode, struct state** values, int size );
void send_header_v1(SSL *ssl, struct request* req, struct response res, struct state** values, int size, int status, const struct sockaddr *cli_addr, socklen_t addr_len);
void send_gif(SSL *ssl, struct request* req, struct response res, struct state** values, int size, struct sockaddr_in	cli_addr, socklen_t addr_len);
void send_string_data(SSL *ssl, struct request* req, struct response res, struct state** values, int i, int size, struct sockaddr_in	cli_addr, socklen_t addr_len);
void send_error(SSL *ssl, struct request* req, struct response res, char* error_msg, struct state** values, int size, struct sockaddr_in	cli_addr, socklen_t addr_len);
int check_valid_query(SSL *ssl, struct request* req, struct response res, char (*keys)[MAXLINE], struct state** values, int size, struct sockaddr_in	cli_addr, socklen_t addr_len);

// ------------------------------main------------------------------
int main(int argc, char	*argv[])
{
    int    sockfd, serv_udp_port;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    struct sockaddr_in	cli_addr, serv_addr;
    SSL_CTX *ctx;

    // check proper input format (general)
    if( argc != 2 ){
        fprintf(stderr, "Usage: %s <port>, where <port> is an integer > 1204 that represents the port to listen on\n", argv[0]);
        exit(-1);
    } else if ( atoi(argv[1]) < 1025) {
        fprintf(stderr, "Port number to low, doesn't have permission to listen. Please enter a port > 1204\n");
        exit(-1);
    }

    serv_udp_port = atoi(argv[1]);

    // configure SSL
    initialize_ssl();
    ctx = create_context(0, 1);
    configure_context(ctx);
    
    // Open a UDP socket (an Internet stream socket).
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        fprintf(stderr, "UDP Server: Can't open stream socket\n");
        exit(-1);
    } else {
        printf("UDP Server: Socket sucessfully created..\n");
    }
    
    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

    // Bind our local address so that the client can send to us.
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port        = htons(serv_udp_port);
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr, "UDP Server: can't bind local address\n");
        exit(-1);
    } else {
        printf("UDP Server: Successfully bound local address..\n");
    }
    
    int size = 0;   // Current number of elements in the map 
    char keys[NUM_STATES][MAXLINE];
    struct state* values[NUM_STATES];
    create_database(TEXT_DATABASE, keys, values, &size);
    printf("UDP Server: Created database...\n");
    
    printf("UDP Server: Running on port %d...\n", serv_udp_port);
    for ( ; ; ) {
        process_request(sockfd, keys, values, size, cli_addr, addr_len, ctx);
    }
    close(sockfd);
    SSL_CTX_free(ctx);
    cleanup_ssl();
    freeMap(values, size);
    return 0;
}

// ------------------------------process_request------------------------------
void process_request(int sockfd, char (*keys)[MAXLINE], struct state** values, int size, struct sockaddr_in	cli_addr, socklen_t addr_len, SSL_CTX *ctx)
{
    char buff[MAXLINE];
    struct response res;
    struct request* req;
    SSL *ssl;
    BIO *bio;

    // Receive data from clients
    int n = recvfrom(sockfd, buff, MAXLINE, 0, (struct sockaddr *)&cli_addr, &addr_len);
    if (n < 0) { // 
        err_dump("UDP Server: process_request, read error", values, size);
    }
    else if (n < sizeof(*req)) {
        err_dump("UDP Server: process_request, request truncated", values, size);
    }
    req = (struct request *)buff;
    // printf("Request: version = %d; opcode = %d; statecode = %s\n", req->version, req->opcode, req->statecode);

    // SSL setup
    ssl = SSL_new(ctx);
    bio = BIO_new_dgram(sockfd, BIO_NOCLOSE);
    BIO_ctrl(bio, BIO_CTRL_DGRAM_SET_CONNECTED, 0, &addr_len);
    SSL_set_bio(ssl, bio, bio);

    if (SSL_accept(ssl) <= 0) {
        perror("UDP Serv: Unable to accept SSL");
        exit(1);
    }

    // send response back -----------------
    int i = check_valid_query(ssl, req, res, keys, values, size, cli_addr, addr_len);
    if ( i != -1 ) {
        printf("UDP Server: valid query...\n");
        if (req->opcode == 5) {
            send_gif(ssl, req, res, values, size, cli_addr, addr_len);
        }
        else {   
            send_string_data(ssl, req, res, values, i, size, cli_addr, addr_len);
        }
        printf("UDP Server: Response sent...\n");
    }
    else {
        printf("UDP Server: Error response sent...\n");
    }

    SSL_shutdown(ssl);
    SSL_free(ssl);
}
// ------------------------------send_header_v1------------------------------
void send_header_v1(SSL *ssl, struct request* req, struct response res, struct state** values, int size, int status, const struct sockaddr *cli_addr, socklen_t addr_len) {
    int n, sz;
    res.version = req->version;
    res.status = status;

    sz = sizeof(res.version) + sizeof(res.status) + sizeof(res.len);  //  1 + 1 + 4
    // n = sendto(sockfd, (void*) &res, sz, 0, cli_addr, addr_len);
    n = SSL_write(ssl, (void*) &res, sz);
    if ( n != sz ){
        err_dump("UDP Server: Error sending response", values, size);
        return;
    }
}

// ------------------------------send_string_data------------------------------
void send_string_data(SSL *ssl, struct request* req, struct response res, struct state** values, int i, int size, struct sockaddr_in	cli_addr, socklen_t addr_len) {
    int n, len;

    // get string and its length
    char* string = query_database(req->statecode, req->opcode, i, values);
    len = strlen(string);
    res.len = htonl(len);

    if (len > MAXLINE) { // if can't fit inside a single UDP packet
        send_error(ssl, req, res, "packet to big, try TCP", values, size, cli_addr, addr_len);
    }
    else {
        // send header
        send_header_v1(ssl, req, res, values, size, 1, (const struct sockaddr *)&cli_addr, addr_len);

        // send data
        // n = sendto(sockfd, (void*) string, len, 0, (const struct sockaddr *)&cli_addr, addr_len);
        n = SSL_write(ssl, (void*) string, len);
        if ( n != len ){
            err_dump("UDP Server: Error sending response", values, size);
            return;
        }
    }
}

// ------------------------------send_gif------------------------------
void send_gif(SSL *ssl, struct request* req, struct response res, struct state** values, int size, struct sockaddr_in	cli_addr, socklen_t addr_len) {
    int n;
    long length;

    // first get filename and open file in rb
    char * file = get_gif_filename(req->statecode, values, size);
    FILE *gifp = fopen(file, "rb");
    if (gifp == NULL) {
        res.status = -1;
        fprintf(stderr, "UDP Server: process_gif: failed to open %s", file);
        free(file);
        freeMap(values, size);
        exit(1);
    }
    // then determine size of file
    fseek(gifp, 0L, SEEK_END);
    length = ftell(gifp);
    rewind(gifp);
    res.len = htonl(length);

    if (length > MAXLINE) { // if can't fit inside a single UDP packet
        send_error(ssl, req, res, "packet to big, try TCP", values, size, cli_addr, addr_len);
    }
    else {
        send_header_v1(ssl, req, res, values, size, 1, (const struct sockaddr *)&cli_addr, addr_len);
        // Read and send the GIF file in chunks
        ssize_t bytes_read;
        char* buffer = (char *)malloc(length); // Allocate memory for buffer
        if (buffer == NULL) {
            perror( "UDP Client: Buffer memory allocation failed" );
            exit(1);
        }

        bytes_read = fread(buffer, 1, length, gifp);
        //n = sendto(sockfd, (void*) buffer, bytes_read, 0, (const struct sockaddr *)&cli_addr, addr_len);
        n = SSL_write(ssl, (void*) buffer, bytes_read);
        if ( n < bytes_read) {
            err_dump("Error sending GIF file", values, size);
        }
    }

    fclose(gifp);
    free(file);
}

// ------------------------------send_error------------------------------
void send_error(SSL *ssl, struct request* req, struct response res, char* error_msg, struct state** values, int size, struct sockaddr_in	cli_addr, socklen_t addr_len) {
    int n, len;

    // get string and its length
    len = strlen(error_msg);
    res.len = htonl(len);

    // send header
    send_header_v1(ssl, req, res, values, size, 255, (const struct sockaddr *)&cli_addr, addr_len);
    
    // send data
    // n = sendto(sockfd, (void*) error_msg, len, 0, (const struct sockaddr *)&cli_addr, addr_len);
    n = SSL_write(ssl, (void*) error_msg, len);
    if ( n != len ){
        err_dump("UDP Server: Error sending response", values, size);
        return;
    }
}

// ------------------------------check_valid_query------------------------------
int check_valid_query(SSL *ssl, struct request* req, struct response res, char (*keys)[MAXLINE], struct state** values, int size, struct sockaddr_in	cli_addr, socklen_t addr_len) {
    printf("UDP Server: Checking query validity...\n");
    // check opcode
    if ( req->opcode > HIGHEST_OPCODE || req->opcode < LOWEST_OPCODE) {
        send_error(ssl, req, res, "invalid opcode", values, size, cli_addr, addr_len);
        return -1;
    }
    // check statecode format
    if ( ! isalpha(req->statecode[0]) || ! isalpha(req->statecode[1]) )  {
        send_error(ssl, req, res, "invalid statecode format", values, size, cli_addr, addr_len);
        return -1;
    }

    // check if real statecode
    char sc[2];
    sc[0] = toupper(req->statecode[0]); // convert statecode to uppercase for query purposes
    sc[1] = toupper(req->statecode[1]);
    int i = getIndex(sc, keys, size);

    if ( i < 0) {
        send_error(ssl, req, res, "statecode does not exist", values, size, cli_addr, addr_len);
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