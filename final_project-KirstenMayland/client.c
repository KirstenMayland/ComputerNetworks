// Kirsten Mayland
// Lab 2 Computer Networks
// Spring 2024

#include  "client.h"

// ------------------------------process_gif------------------------------
void process_gif(char statecode[2], int len, char* buffer) {

    // create file name
    char *str = malloc(strlen(statecode) + strlen(".gif") + 1); // +1 for the null terminator
    if (str == NULL) {
        fprintf(stderr, "process_gif: memory allocation failed\n");
        return;
    }
    strcpy(str, statecode);
    strcat(str, ".gif");
    
    FILE *gifp = fopen(str, "w");
    if (gifp == NULL) {
        fprintf(stderr, "process_gif: failed to open %s\n", str);
        return;
    }
    fwrite( buffer, len, 1, gifp);

    fclose(gifp);

    free(str); // Don't forget to free dynamically allocated memory
}