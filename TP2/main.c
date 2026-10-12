#include "malloc.h"
#include <stdio.h>

int main(){
    //Contruire liste
    void *test = malloc_3is(0);
    printf("Allocation : adresse = %p \n", test);
    void *test1 = malloc_3is(10);
    printf("Allocation : adresse = %p \n", test1);
    void *test2 = malloc_3is(500);
    printf("Allocation : adresse = %p \n", test2);
    void *test3 = malloc_3is(9999999);
    printf("Allocation : adresse = %p \n", test3);
    return 0;
}