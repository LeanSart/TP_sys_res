#include "malloc.h"
#include <stdio.h>
HEADER *first_header = NULL;

void *malloc_3is(size_t block_size){
    if(block_size == 0){ //Renvoie NULL si la taille du bloc demandé est de 0
        return NULL;
    }
    HEADER *noeud = first_header;
    while(noeud != NULL){ //On regarde s'il existe un bloc de la bonne taille ou plus grand pour notre nouveau bloc
        if(noeud->bloc_size >= block_size){
            void *block_ptr = (void *)(noeud + 1);
            
            size_t reste = noeud->bloc_size - block_size;
            size_t espace_min = sizeof(HEADER) + sizeof(long) + 1;

            if(reste >= espace_min){ //Si un espace plus grand existe, on le sort de la liste cahainée puis on scinde le bloc en 2, 
                //on sort la partie utile de la liste chainée et on remet la partie inutilisée dans la liste chainée
                noeud->bloc_size = block_size;
                long *magic_ptr = (long *)((char *)block_ptr + block_size);
                *magic_ptr = MAGIC_NUMBER;


                HEADER *reste_noeud = (HEADER *)((char *)magic_ptr + sizeof(long));
                reste_noeud->bloc_size = reste - sizeof(HEADER) - sizeof(long);
                reste_noeud->magic_number = MAGIC_NUMBER;
                reste_noeud->ptr_next = NULL;
                
                void *reste_block_ptr = (void *)(reste_noeud + 1);
                long *reste_magic_ptr = (long *)((char *)reste_block_ptr + reste_noeud->bloc_size);
                *reste_magic_ptr = MAGIC_NUMBER;

                supprnoeud(block_ptr);
                free_3is(reste_block_ptr);
            } else { //Si on trouve un espace de la bonne taille, on sort juste le bloc de la liste chainée
                supprnoeud(block_ptr);
            }
            return block_ptr;
        }
        noeud = noeud->ptr_next;
    }
    size_t pre_alloc_block_size = block_size;
    if (pre_alloc_block_size < MIN_BLOC_SIZE) { //On pré-alloue une taille minimum
        pre_alloc_block_size = MIN_BLOC_SIZE;
    }
    void *total =  sbrk(sizeof(HEADER) + pre_alloc_block_size + sizeof(MAGIC_NUMBER)); //Si aucun bloc de la bonne taille n'existe déjà on en crée un nouveau sur le tas
    if(total == (void *)-1){ //Renvoie NULL s'il n'y a plus d'espace mémoire système
        return NULL;
    }
    noeud = (HEADER *)total;
    noeud->bloc_size = pre_alloc_block_size;
    noeud->magic_number = MAGIC_NUMBER;
    noeud->ptr_next = NULL;
    void *block_ptr = (void *)(noeud + 1);
    long *magic_ptr = (long *)((char *)block_ptr + pre_alloc_block_size); //On caste en long et on se place de block_size valeurs après le pointeur du bloc
    *magic_ptr = MAGIC_NUMBER; 
    if (pre_alloc_block_size > block_size) {//Si on a trop pré-alloué, on libère tout le bloc puis on ré-appel malloc 
        free_3is(block_ptr);
        return malloc_3is(block_size); 
    }
    return block_ptr; //Renvoie le pointeur du bloc si on a alloué pile la bonne taille
}


void free_3is(void *ptr){ //Modification apportée : lors de la libération on insère le nouvel élément au bon endroit dans la liste pour que la liste soit toujours triée par ordre croissant d'adresses
    if(ptr == NULL){
        return;
    }
    HEADER *precedent = NULL;
    HEADER *header = ((HEADER *)ptr)-1;
    if(first_header == NULL || header < first_header){
        header->ptr_next = first_header;
        first_header = header;
    }else{
        HEADER *noeud = first_header;
        while(noeud->ptr_next != NULL && noeud->ptr_next < header){
            noeud = noeud->ptr_next;
        }
        precedent = noeud; //initialisation de precedent en cas de fusion avec le bloc precedent
        header->ptr_next = noeud->ptr_next;
        noeud->ptr_next = header;
    }
    if(header->ptr_next != NULL){//A chaque liberation de bloc on fusionne les blocs adjacents si necessaire
        char *fin_bloc_actuel = (char *)header + sizeof(HEADER) + header->bloc_size + sizeof(long);
        if(fin_bloc_actuel == (char *)header->ptr_next){ //Verification si notre bloc actuel est suivi d'un autre bloc de la liste chainée
            header->bloc_size += sizeof(HEADER) + header->ptr_next->bloc_size + sizeof(long); //On ajoute dans le bloc l'ensemble du bloc suivant
            header->ptr_next = header->ptr_next->ptr_next;
        }
    }
    if(precedent != NULL){// On fait pareil mais avec le bloc precedent
        char *fin_bloc_precedent = (char *)precedent + sizeof(HEADER) + precedent->bloc_size + sizeof(long);
        if(fin_bloc_precedent == (char *)header){
            precedent->bloc_size += sizeof(HEADER) + header->bloc_size + sizeof(long); //On ajoute dans le bloc precedent l'ensemble du bloc
            precedent->ptr_next = header->ptr_next;
        }
    }
}

int afficheliste(){ //Affiche toute la liste chainée à partir du noeud de tete jusqu'au noeud de fin
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

int checkblock(void *ptr){ //Renvoie 0 : Aucun débordement renvoie 1 : débordement en amont 
// renvoie 2 : débordement en aval renvoie 3 : débordement en amont et en aval
    if(ptr == NULL){
        return -1;
    }
    HEADER *header = ((HEADER *)ptr)-1;
    long *magic_ptr = (long *)((char *)ptr + header->bloc_size);
    if(header->magic_number != MAGIC_NUMBER){
        if(*magic_ptr != MAGIC_NUMBER){
            return 3;
        }
        return 1;
    }
    else if(*magic_ptr != MAGIC_NUMBER){
        return 2;
    }
    return 0;
}

void supprnoeud(void *ptr){
    if((ptr == NULL) || (first_header == NULL)){
        return;
    }
    HEADER *header = ((HEADER *)ptr)-1;
    if(header == first_header){
        first_header = header->ptr_next;
    }
    else{
        HEADER *prev_header = first_header;
        while(prev_header->ptr_next != header){
            prev_header = prev_header->ptr_next;
        }
        prev_header->ptr_next = header->ptr_next;
    }
    header->ptr_next = NULL;
}