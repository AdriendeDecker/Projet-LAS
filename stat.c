#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "utils_v10.h"
#include "typeDefStruct.h"

#define SHMKEY_PROGRAMMES 123
#define SHMKEY_INDEX 456
#define SEMKEY 789


int main(int argc, char const *argv[]) {
    if(argc < 2){
        printf("Il manque un paramètre ! \n");
        return 0;
    }
    
    int sem_id, shm_id_prog;
    
    //Get Shared Memory + Semaphore
    sem_id = sem_get(SEMKEY, 1);
    shm_id_prog = sshmget(SHMKEY_PROGRAMMES, sizeof(program[1000]), 0);

    program* programs;
    programs = sshmat(shm_id_prog);

    //print out info about program
    int id = atoi(argv[1]);
    printf("%d\n", id);
    printf("%s\n", programs[id].name);
    if(programs[id].compiled){
        printf("true\n");
    }else{
        printf("false\n");
    }
    printf("%d\n",programs[id].executedCount);
    printf("%f\n",programs[id].durationMS);

    sshmdt(programs);

    return 0;
}
