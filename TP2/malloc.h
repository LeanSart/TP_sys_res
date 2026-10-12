
#include <stdlib.h>
#include <unistd.h>

#define MIN_BLOC_SIZE 1000
#define MAGIC_NUMBER ((long)0xBADA110CBADA110CL)

typedef struct HEADER_TAG {
    struct HEADER_TAG * ptr_next; /* pointe sur le prochain bloc libre */
    size_t bloc_size; /* taille du memory bloc en octets*/
    long magic_number; /* 0xBADA110CBADA110CL */
} HEADER;

extern HEADER *first_header;

void *malloc(size_t size);
void free(void *ptr);
void *malloc_3is(size_t block_size);
void free_3is(void *ptr);
int afficheliste();
void supprnoeud(void *ptr);