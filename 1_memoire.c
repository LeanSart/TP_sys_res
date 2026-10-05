#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

int data_var = 42;
int bss_var;

void exec_command(char *const argv[]){ //Fonction pour executer proprement une commande système
    pid_t pid = fork();
    if (pid == -1) {
        perror("Erreur lors du fork");
        exit(1);
    } else if (pid == 0) {
        execvp(argv[0],argv);
        perror("Erreur execvp");
        exit(1);
    } else {
        wait(NULL);
    }
}


int main() {
    int stack_var = 10;
    const char *str_var = "Hello World!";
    void *heap_var = malloc(1024);
    void *mmap_var = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    
    printf("%-30s : %p\n", "Data (Global initialise)", (void*)&data_var);
    printf("%-30s : %p\n", "BSS (Global non initialise)", (void*)&bss_var);
    printf("%-30s : %p\n", "Str (Chaine rodata)", (void*)str_var);
    printf("%-30s : %p\n", "Heap (Allocation malloc)", heap_var);
    printf("%-30s : %p\n", "Stack (Variable locale)", (void*)&stack_var);
    printf("%-30s : %p\n", "Main Function (.text)", (void*)main);
    printf("%-30s : %p\n", "LibC Function (printf)", (void*)printf);
    printf("%-30s : %p\n", "Mmap (Allocation mmap)", mmap_var);

    char pid_str[32]; 
    snprintf(pid_str, sizeof(pid_str), "%d", getpid());
    char *const argv_pmap[] = {"pmap","-X",pid_str,NULL};
    exec_command(argv_pmap);
    return 0;
}