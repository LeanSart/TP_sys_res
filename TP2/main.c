#include "malloc.h"
#include <stdio.h>

int main(){
    void *test = malloc_3is(0); //multiples allocations
    printf("Allocation : adresse = %p \n", test);
    void *test1 = malloc_3is(10);
    printf("Allocation : adresse = %p \n", test1);
    void *test2 = malloc_3is(500);
    printf("Allocation : adresse = %p \n", test2);
    void *test3 = malloc_3is(999);
    printf("Allocation : adresse = %p \n", test3);
    printf("size header %ld \n", sizeof(HEADER));
    printf("size Magic number %ld \n", sizeof(MAGIC_NUMBER));
    afficheliste();
    free_3is(test1); //libérations de blocs
    printf("Liste chainée des bloc libérés : adresse de tête = %p \n", first_header);
    afficheliste();
    free_3is(test2); //On remarque que la fusion s'effectue bien
    printf("Liste chainée des bloc libérés : adresse de tête = %p \n", first_header);
    afficheliste();
    free_3is(test3);
    printf("Liste chainée des bloc libérés : adresse de tête = %p \n", first_header);
    afficheliste();
    void *test4 = malloc_3is(10); //Réutilisation d'un bloc libéré
    printf("Allocation : adresse = %p \n", test4);
    afficheliste();
    void *test5 = malloc_3is(300); //Réutilisation d'un bloc libéré avec découpage en deux plus petit blocs
    printf("Allocation : adresse = %p \n", test5);
    afficheliste();
    return 0;
}   