#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/wait.h>

void exec_command(char *const argv[]){  //Fonction pour executer proprement une commande système
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
    char *const argv_cat[] = {"cat", "test.txt", NULL};
    char *const argv_stat[] = {"stat", "test.txt", NULL};
    printf("Fichier original : \n");
    exec_command(argv_cat);
    printf("\n");
    exec_command(argv_stat);
    int file = open("test.txt", O_RDWR);
    char *mmap_var = mmap(NULL,206,PROT_READ | PROT_WRITE,MAP_SHARED, file, 0); //Mapping du fichier
    for(int i = 0; i < (206/2); i++){ // Inversion des octets du fichier
        char temp = mmap_var[i];
        mmap_var[i] = mmap_var[206 - i];
        mmap_var[206 - i] = temp;
    }
    munmap(mmap_var,206); //Unmapping du fichier
    close(file);
    printf("Fichier inversé : \n");
    exec_command(argv_cat);
    printf("\n");
    return 0;
}
