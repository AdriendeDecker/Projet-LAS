#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "utils_v10.h"

#define SHMKEY 123
#define SEMKEY 456

typedef struct PROGRAM {
    char name[255];
    bool compiled;
    int executedCount;
    int durationMS;
} program;

int main(int argc, char const *argv[]) {
    if(argc < 2){
        printf("Il manque un paramètre ! \n");
        return 0;
    }
    
    int sem_id, shm_id;
    
    sem_id = sem_get(SEMKEY, 1);
    shm_id = sshmget(SHMKEY, sizeof(program[1000]), 0);

    program* programs;

    programs = sshmat(shm_id);

    
    int id = atoi(argv[1])-1;
    printf("%d\n", id+1);
    printf("%s\n", programs[id].name);
    if(programs[id].compiled){
        printf("true\n");
    }else{
        printf("false\n");
    }
    printf("%d\n",programs[id].executedCount);
    printf("%d\n",programs[id].durationMS);

    sshmdt(programs);

    return 0;
}
