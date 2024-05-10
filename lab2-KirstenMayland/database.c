// Kirsten Mayland
// Lab 2 Computer Networks
// Spring 2024

#include  "database.h"

// ------------------------------create_database------------------------------
void create_database(char* database_loc, char (*keys)[MAXLINE], struct state** values, int* size) {
    FILE* fp = fopen(database_loc, "r");
    if (fp == NULL) {
        fprintf(stderr, "TCP Server: create_database: failed to open %s\n", database_loc);
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
        insert(abrev, data, keys, values, size);
    }

    fclose(fp);
}
  
// ------------------------------getIndex------------------------------
int getIndex(char key[], char (*keys)[MAXLINE], int size) { 
    for (int i = 0; i < size; i++) { 
        if (strcmp(keys[i], key) == 0) { 
            return i; 
        } 
    } 
    return -1; // Key not found 
} 

// ------------------------------insert------------------------------  
void insert(char key[], struct state* value, char (*keys)[MAXLINE], struct state** values, int* size) { 
    int index = getIndex(key, keys, *size); 
    if (index == -1) { // Key not found 
        strcpy(keys[*size], key); 
        values[*size] = value; 
        *size +=1;
    } 
    else { // Key found 
        values[index] = value; 
    } 
} 
  
// ------------------------------printMap------------------------------  
void printMap(char (*keys)[MAXLINE], struct state** values, int size) { 
    for (int i = 0; i < size; i++) { 
        printf("%s: %s -- %s -- %s -- %s\n", keys[i], values[i]->name, values[i]->capital, values[i]->date, values[i]->motto); 
    } 
} 
  
// ------------------------------freeMap------------------------------  
void freeMap(struct state** values, int size) {
    for (int i = 0; i < size; i++) {
        free(values[i]->name);
        free(values[i]->capital);
        free(values[i]->date);
        free(values[i]->motto);
        free(values[i]);
    } 
}