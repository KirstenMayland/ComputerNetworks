// Kirsten Mayland
// Lab 2 Computer Networks
// Spring 2024

#define MAXLINE 1024
#define RESULT_STATUS_OK   1
#define HIGHEST_OPCODE  5
#define LOWEST_OPCODE   1

struct request {
     uint8_t   version;
     uint8_t   opcode;
     char statecode[2];
} __attribute__((packed));

struct response {
     uint8_t   version;
     uint8_t   status;
     uint32_t  len;
} __attribute__((packed));

struct request2 {
     uint8_t   version;
     uint8_t   num_queries;
} __attribute__((packed));

struct response2 {
     uint8_t   version;
     uint8_t   status;
     uint8_t   num_queries;
     uint8_t   reserved;
} __attribute__((packed));

struct query2 {
     uint8_t   opcode;
     char statecode[2];
} __attribute__((packed));

struct state {
     char* name;
     char* capital;
     char* date;
     char* motto;
};