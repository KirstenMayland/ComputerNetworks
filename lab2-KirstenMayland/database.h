// Kirsten Mayland
// Lab 2 Computer Networks
// Spring 2024

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

#define NUM_STATES      50
#define TEXT_DATABASE   "data/statedb.txt"
#define FLAGS_LOC       "data/flags/"


// ------------------------------create_database------------------------------
void create_database(char* database_loc, char (*keys)[MAXLINE], struct state** values, int* size);
  
// ------------------------------getIndex------------------------------
// Function to get the index of a key in the keys array 
// Credit: https://www.geeksforgeeks.org/implementation-on-map-or-dictionary-data-structure-in-c/#
int getIndex(char key[], char (*keys)[MAXLINE], int size);

// ------------------------------insert------------------------------  
// Function to insert a key-value pair into the map 
// Credit: https://www.geeksforgeeks.org/implementation-on-map-or-dictionary-data-structure-in-c/#
void insert(char key[], struct state* value, char (*keys)[MAXLINE], struct state** values, int* size);
  
// ------------------------------printMap------------------------------  
// Credit: https://www.geeksforgeeks.org/implementation-on-map-or-dictionary-data-structure-in-c/#
void printMap(char (*keys)[MAXLINE], struct state** values, int size);
  
// ------------------------------freeMap------------------------------  
void freeMap(struct state** values, int size);