
#include <stdlib.h>
#include <unistd.h>

#define MAGIC_NUMBER 0xBADA110CBADA110CL

typedef struct HEADER_TAG {
    struct HEADER_TAG * ptr_next; /* pointe sur le prochain bloc libre */
    size_t bloc_size; /* taille du memory bloc en octets*/
    long magic_number; /* 0xBADA110CBADA110CL */
} HEADER;

static HEADER *first_header = NULL;

void *malloc_3is(size_t block_size);
void free_3is(void *ptr);