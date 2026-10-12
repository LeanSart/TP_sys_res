#include "malloc.h"
#include <stdio.h>
HEADER *first_header = NULL;

void *malloc_3is(size_t block_size){
    if(block_size == 0){ //Renvoie NULL si la taille du bloc demandé est de 0
        return NULL;
    }
    void *total =  sbrk(sizeof(HEADER) + block_size + sizeof(MAGIC_NUMBER));
    if(total == (void *)-1){ //Renvoie NULL s'il n'y a plus d'espace mémoire système
        return NULL;
    }
    HEADER *noeud = (HEADER *)total;
    noeud->bloc_size = block_size;
    noeud->magic_number = MAGIC_NUMBER;
    noeud->ptr_next = NULL;
    void *block_ptr = (void *)(noeud + 1);
    long *magic_ptr = (long *)((char *)block_ptr + block_size); //On caste en long et on se place de block_size valeurs après le pointeur du bloc
    *magic_ptr = MAGIC_NUMBER; 
    return block_ptr; //Renvoie le pointeur du bloc
}



void free_3is(void *ptr){
    if(ptr == NULL){
        return;
    }
    HEADER *header = ((HEADER *)ptr)-1;
    if(first_header == NULL){
        first_header = header;
    }else{
        HEADER *noeud = first_header;
        while(noeud->ptr_next != NULL){
            noeud = noeud->ptr_next;
        }
        noeud->ptr_next = header;
    }
}

int afficheliste(){ //Affiche toute la lsite chainée à partir du noeud de tete jusqu'au noeud de fin
    HEADER *noeud = first_header;
    if(noeud == NULL){
        printf("Liste vide\n");
        return -1;
    }
    while(noeud->ptr_next != NULL){
        printf("<%p>:%ld\n",(void*)noeud,noeud->bloc_size);
        noeud = noeud->ptr_next;
    }
    printf("<%p>:%ld\n",(void*)noeud,noeud->bloc_size);
    return 0;
}

/*
HEADER getTete(){
    if(first_header == NULL){
        first_header = creerTete;
    }
    return *first_header;
}

HEADER creerTete(){
    first_header->bloc_size = ;
    first_header->ptr_next = NULL;
    first_header->magic_number = MAGIC_NUMBER; 
}

*/



