// Kirsten Mayland
// Final Project Computer Networks
// Spring 2024

#include  "authentication.h"

// ------------------------------initialize_ssl------------------------------
void initialize_ssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

// ------------------------------cleanup_ssl------------------------------
void cleanup_ssl() {
    EVP_cleanup();
}

// ------------------------------create_context------------------------------
SSL_CTX* create_context(int side, int type) {
    const SSL_METHOD *method;
    SSL_CTX *ctx;
    if (side > 1 || side < 0 || type > 1 || type < 0) {
        perror("Incorrect side and/or type in create_context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (type == 0) { // TCP
        if (side == 0){
            method = SSLv23_server_method();
        } else if (side == 1){
            method = SSLv23_client_method();
        }
    } else if (type == 1) { // UDP
        if (side == 0){
            method = DTLS_server_method();
        } else if (side == 1){
            method = DTLS_client_method();
        }
    }

    ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

// ------------------------------configure_context------------------------------
void configure_context(SSL_CTX *ctx) {
    if (SSL_CTX_use_certificate_file(ctx, "server.crt", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

// ------------------------------info_callback------------------------------
void info_callback(const SSL *ssl, int where, int ret) {
    // for debugging purposes
    const char *str;
    int w = where & ~SSL_ST_MASK;

    // which func to debug
    if (w & SSL_ST_CONNECT) str = "SSL_connect";
    else if (w & SSL_ST_ACCEPT) str = "SSL_accept";
    else str = "undefined";

    // what errors its throwing
    if (where & SSL_CB_LOOP)
        printf("%s:%s\n", str, SSL_state_string_long(ssl));
    else if (where & SSL_CB_ALERT) {
        printf("SSL3 alert %s:%s:%s\n", (where & SSL_CB_READ) ? "read" : "write",
                    SSL_alert_type_string_long(ret),
                    SSL_alert_desc_string_long(ret));
    } else if (where & SSL_CB_EXIT) {
        if (ret == 0)
            printf("%s:failed in %s\n", str, SSL_state_string_long(ssl));
        else if (ret < 0)
            printf("%s: error in %s\n", str, SSL_state_string_long(ssl));
    }
}
