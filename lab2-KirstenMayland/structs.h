

#define MAXLINE 255

/* 
 *  Without "packed" the compiler will insert 
 *   padding into the struct. Remove it and 
 *   see what happens; printf( "%p %p\n", &req.pow, &req.num )
 *   will show you what the pointer values are.
 */    

#define RESULT_STATUS_OK   1
#define HIGHEST_OPCODE  5
#define LOWEST_OPCODE   1

struct request {
     uint8_t version;
     uint8_t  opcode;
     char statecode[2];
} __attribute__((packed));

struct response {
     uint8_t version;
     uint8_t  status;
     uint32_t len;
     char     str[MAXLINE];
} __attribute__((packed));

struct state {
     char* name;
     char* capital;
     char* date;
     char* motto;
};