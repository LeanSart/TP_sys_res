#include "segdef.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

void init_rand(){
    srand(time(NULL) ^ getpid());
}

long getrand(){
    return (long)rand();
}

void wait_sem(int semid,int sem){
    struct sembuf sop;

    sop.sem_num = sem;
    sop.sem_op  = 0;
    sop.sem_flg = 0;

    while (semop(semid, &sop, 1) == -1) {
        if (errno == EINTR)
            continue;

        perror("wait_sem");
    }
}

void acq_sem(int semid,int sem){
    struct sembuf sop;

    sop.sem_num = sem;
    sop.sem_op  = -1;
    sop.sem_flg = 0;

    while (semop(semid, &sop, 1) == -1) {
        if (errno == EINTR)
            continue;

        perror("acq_sem");
    }
}
void lib_sem(int semid,int sem){
    struct sembuf sop;

    sop.sem_num = sem;
    sop.sem_op  = +1;
    sop.sem_flg = 0;

    while (semop(semid, &sop, 1) == -1) {
        if (errno == EINTR)
            continue;

        perror("lib_sem");
    }
}