#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "utils_v10.h"

#define SHMKEY_PROGRAMMES 123
#define SHMKEY_INDEX 456
#define SEMKEY 789

typedef struct PROGRAM {
    char name[255];
    bool compiled;
    int executedCount;
    float durationMS;
} program;

int main(int argc, char const *argv[]) {
    if(argc < 2){
        printf("Il manque un paramètre ! \n");
        return 0;
    }
    
    int sem_id, shm_id_prog, shm_id_index;
    
    sem_id = sem_get(SEMKEY, 1);
    shm_id_prog = sshmget(SHMKEY_PROGRAMMES, sizeof(program[1000]), 0);
    shm_id_index = sshmget(SHMKEY_INDEX, sizeof(int), 0);

    program* programs;
    int* programsIndex;

    programs = sshmat(shm_id_prog);
    programsIndex = sshmat(shm_id_index);

    
    int id = atoi(argv[1])-1;
    printf("%d\n", id+1);
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
