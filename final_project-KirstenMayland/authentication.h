// Kirsten Mayland
// Final Project Computer Networks
// Spring 2024

#include  <stdio.h>
#include  <sys/types.h>
#include  <sys/socket.h>
#include  <netinet/in.h>
#include  <arpa/inet.h>
#include  <openssl/ssl.h>
#include  <openssl/err.h>
#include  <stdio.h>
#include  <strings.h> // bzero
#include  <string.h>  // bzero
#include  <stdlib.h>  // exit
#include  <unistd.h>  // close
#include  <stdint.h>  // uint8_t
#include  <ctype.h>


// ------------------------------initialize_ssl------------------------------
void initialize_ssl();

// ------------------------------cleanup_ssl------------------------------
void cleanup_ssl();

// ------------------------------create_context------------------------------
// side: 0 = server, 1 = client
// type: 0 = TCP, 1 = UDP
SSL_CTX* create_context(int side, int type);

// ------------------------------configure_context------------------------------
void configure_context(SSL_CTX *ctx);

// ------------------------------info_callback------------------------------
// for debugging purposes
void info_callback(const SSL *ssl, int where, int ret);