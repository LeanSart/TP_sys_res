#include "segdef.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
 
struct sembuf sop ;

void client_init(segment *seg){
    init_rand();
    for(int i = 0;i < maxval; i++){
        seg->tab[i] = getrand();
    }
}
long moyenne(long tab[]) {
    long total=0;
    for (size_t i = 0; i < maxval; i++) {
        total += tab[i];
    }
    return total / maxval;
}

void boucle(segment *seg, int semid){
    acq_sem(semid,seg_dispo);
    client_init(seg);
    long mean = moyenne(seg->tab);
    acq_sem(semid,seg_init);
    wait_sem(semid,res_ok);
    long result = seg->result;
    lib_sem(semid,seg_init);
    acq_sem(semid,res_ok);
    lib_sem(semid,res_ok);
    lib_sem(semid,seg_dispo);
    if(result == mean){
        printf("[Client %d] resultat serveur\t: %ld\nresultat client\t\t: %ld\r\n",getpid(), result, mean);
    }
}
    
int main(){

    int nb_clients = 5; 

    int shmid = shmget(cle, sizeof(segment), 0666);
    if (shmid == -1) {
        perror("shmget");
        return 1;
    }
    segment *seg = (segment *) shmat(shmid, NULL, 0);
    if (seg == (void *) -1) {
        perror("shmat");
        return 1;
    }
    int semid = semget(cle, 3, 0666);
    if (semid == -1) {
        perror("semget");
        return 1;
    }
    
    for (int i = 0; i < nb_clients; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("Erreur lors du fork");
            exit(1);
        } else if (pid == 0) {
            boucle(seg, semid);
            if (shmdt(seg) == -1) {
                perror("shmdt");
                return 1;
            }
            exit(1);
        } else {
            wait(NULL);
        }
    }
    return 0;
}