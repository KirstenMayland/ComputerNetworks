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
#include  <string.h>  // bzero
#include  <stdlib.h>  // exit
#include  <unistd.h>  // close
#include  <stdint.h>  // uint8_t
#include  <ctype.h>
#include "structs.h"

#define SERV_HOST_ADDR  "129.170.212.8"

// creating database
#define NUM_STATES      50
int size = 0;   // Current number of elements in the map 
char keys[NUM_STATES][MAXLINE];
struct state* values[NUM_STATES];
char* text_database = "data/statedb.txt";
char* flags_loc = "data/flags/";

// local functions
void err_dump(char *);
void process_request(int sockfd);
char* get_gif_filename( char* statecode );
void send_gif(int sockfd, struct request* req, struct response res);
void send_string_data(int sockfd, struct request* req, struct response res, int i);
void send_error(int sockfd, struct request* req, struct response res, char* error_msg);
int check_valid_query(int sockfd, struct request* req, struct response res);
char* query_database(char* statecode, int opcode, struct response *res, int i);
void create_database();
int getIndex(char key[]);
void insert(char key[], struct state* value);
void printMap();
void freeMap();

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
        fprintf(stderr, "TCP Server: Can't open stream socket\n");
        exit(-1);
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
        fprintf(stderr, "TCP Server: can't bind local address\n");
        exit(-1);
    } else {
        printf("TCP Server: Successfully bound local address..\n");
    }
    
    listen(sockfd, 5);

    create_database();
    printf("TCP Server: Created database...\n");
    //printMap();
    
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
            printf("TCP Server: In child, calling process_request...\n");
            process_request(newsockfd);  // process the request, loops until peer closes connection
            exit(0);
        }
            
        close(newsockfd); // parent process
    }
    freeMap();
}

// ------------------------------process_request------------------------------
void process_request(int sockfd) {
    int n;
    struct response res;
    struct request* req;
    char buff[MAXLINE];

    // receive message from client -----------------
    n = read(sockfd, buff, MAXLINE); 
    if (n < 0) { // 
        err_dump("TCP Server: process_request, read error");
    }
    else if (n < sizeof(*req)) {
        err_dump("TCP Server: process_request, request truncated");
    }
    req = (struct request *)buff;


    // send response back -----------------
    res.version = req->version;
    res.status = 1;

    int i = check_valid_query(sockfd, req, res);
    if ( i != -1 ) {
        if (req->opcode == 5) {
            send_gif(sockfd, req, res);
        }
        else {   
            send_string_data(sockfd, req, res, i);
        }
        printf("TCP Server: Response sent...\n");
    }
    else {
        printf("TCP Server: Error response sent...\n");
    }
}

// ------------------------------send_string_data------------------------------
void send_string_data(int sockfd, struct request* req, struct response res, int i) {
    int n, len, size;

    // get string and its length
    char* string = query_database(req->statecode, req->opcode, &res, i);
    len = strlen(string);
    res.len = htonl(len);

    // send header
    size = sizeof(res.version) + sizeof(res.status) + sizeof(res.len);  //  1 + 1 + 4
    n = write(sockfd, (void*) &res, size);
    if ( n != size ){
        err_dump("TCP Server: Error sending response");
        return;
    }
    // send data
    n = write(sockfd, (void*) string, len);
    if ( n != len ){
        err_dump("TCP Server: Error sending response");
        return;
    }
}

// ------------------------------send_gif------------------------------
void send_gif(int sockfd, struct request* req, struct response res) {
    int n, size;
    long length;

    // first get filename and open file in rb
    char * file = get_gif_filename(req->statecode);
    FILE *gifp = fopen(file, "rb");
    if (gifp == NULL) {
        res.status = -1;
        fprintf(stderr, "TCP Server: process_gif: failed to open %s", file);
        free(file);
        freeMap();
        exit(1);
    }
    // then determine size of file
    fseek(gifp, 0L, SEEK_END);
    length = ftell(gifp);
    rewind(gifp);

    // add length to header and send header
    res.len = htonl(length);
    size = sizeof(res.version) + sizeof(res.status) + sizeof(res.len);  //  1 + 1 + 4
    n = write(sockfd, (void*) &res, size);
    if ( n != size ){
        err_dump("TCP Server: Error sending response");
        return;
    }

    // Read and send the GIF file in chunks
    ssize_t bytes_read;
    char buffer[1024];
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), gifp)) > 0) {
        if (send(sockfd, buffer, bytes_read, 0) < 0) {
            err_dump("Error sending GIF file");
        }
    }
    fclose(gifp);
    free(file);
}

// ------------------------------send_error------------------------------
void send_error(int sockfd, struct request* req, struct response res, char* error_msg) {
    int n, len, size;

    res.status = 255;

    // get string and its length
    len = strlen(error_msg);
    res.len = htonl(len);

    // send header
    size = sizeof(res.version) + sizeof(res.status) + sizeof(res.len);  //  1 + 1 + 4
    n = write(sockfd, (void*) &res, size);
    if ( n != size ){
        err_dump("TCP Server: Error sending response");
        return;
    }
    // send data
    n = write(sockfd, (void*) error_msg, len);
    if ( n != len ){
        err_dump("TCP Server: Error sending response");
        return;
    }
}


// ------------------------------query_database------------------------------
char* query_database(char* statecode, int opcode, struct response *res, int i) {

    if (opcode == 1) {
        return values[i]->name;
    } else if (opcode == 2) {
        return values[i]->capital;
    } else if (opcode == 3) {
        return values[i]->date;
    } else if (opcode == 4) {
        return values[i]->motto;  
    }
    return "ERROR";
}

// ------------------------------get_gif_filename------------------------------
char* get_gif_filename( char* statecode ) {
    // create file name
    char sc[2];
    sc[0] = tolower(statecode[0]);  // convert statecode to lowercase for query purposes
    sc[1] = tolower(statecode[1]);
    char *file = malloc(strlen(flags_loc) + strlen(sc) + strlen(".gif") + 1); // +1 for the null terminator
    if (file == NULL) {
        err_dump("TCP Server: get_gif: memory allocation failed");
    }
    strcpy(file, flags_loc);
    strcat(file, sc);
    strcat(file, ".gif");

    return file;
}

// ------------------------------check_valid_query------------------------------
int check_valid_query(int sockfd, struct request* req, struct response res) {
    printf("TCP Server: Checking query validity...\n");
    // check opcode
    if ( req->opcode > HIGHEST_OPCODE || req->opcode < LOWEST_OPCODE) {
        send_error(sockfd, req, res, "invalid opcode");
        return -1;
    }
    // check statecode format
    if ( ! isalpha(req->statecode[0]) || ! isalpha(req->statecode[1]) )  {
        send_error(sockfd, req, res, "invalid statecode format");
        return -1;
    }

    // check if real statecode
    char sc[2];
    sc[0] = toupper(req->statecode[0]); // convert statecode to uppercase for query purposes
    sc[1] = toupper(req->statecode[1]);
    int i = getIndex(sc);

    if ( i < 0) {
        send_error(sockfd, req, res, "statecode does not exist");
        return -1;
    }

    return i;  // if statecode is in database, get consequent data
}

// ------------------------------err_dump------------------------------
void err_dump(char *msg)
{
    perror(msg);
    freeMap();
    exit(1);
}


// ******************************************************************************
// ******************************database functions******************************
// ******************************************************************************

// ------------------------------create_database------------------------------
void create_database() {
    FILE* fp = fopen(text_database, "r");
    if (fp == NULL) {
        fprintf(stderr, "TCP Server: create_database: failed to open %s\n", text_database);
        exit(1);
    }

    char line[MAXLINE];
    // go through each line in the data file
    while (fgets(line, MAXLINE, fp) != NULL) {
        char *token = strtok(line, "|");
        char *abrev = token;

        int t = 1;
        struct state *data = malloc (sizeof(struct state)); 
        // split the line up into tokens and store them respectively
        while (token != NULL) {
            if (t == 2) {
                data->name = malloc(strlen(token) + 1); 
                strcpy(data->name, token);
            } else if (t == 3) {
                data->capital = malloc(strlen(token) + 1);
                strcpy(data->capital, token);
            } else if (t == 4) {
                data->date = malloc(strlen(token) + 1); 
                strcpy(data->date, token);
            } else if (t == 5) { 
                token[strcspn(token, "\n")] = '\0'; // remove trailing new line char
                data->motto = malloc(strlen(token) + 1);
                strcpy(data->motto, token);          
            }

            token = strtok(NULL, "|");
            t += 1;
        }
        // store this data as a new key-value pair
        insert(abrev, data);
    }

    fclose(fp);
}
  
// ------------------------------getIndex------------------------------
// Function to get the index of a key in the keys array 
// Credit: https://www.geeksforgeeks.org/implementation-on-map-or-dictionary-data-structure-in-c/#
int getIndex(char key[]) { 
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
void insert(char key[], struct state* value) { 
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
  
// ------------------------------printMap------------------------------  
// Credit: https://www.geeksforgeeks.org/implementation-on-map-or-dictionary-data-structure-in-c/#
void printMap() { 
    for (int i = 0; i < size; i++) { 
        printf("%s: %s -- %s -- %s -- %s\n", keys[i], values[i]->name, values[i]->capital, values[i]->date, values[i]->motto); 
    } 
} 
  
// ------------------------------freeMap------------------------------  
void freeMap() {
    for (int i = 0; i < size; i++) {
        free(values[i]->name);
        free(values[i]->capital);
        free(values[i]->date);
        free(values[i]->motto);
        free(values[i]);
    } 
}