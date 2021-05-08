#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "utils_v10.h"

#define SHMKEY 123
#define SEMKEY 456
#define  PERM 0666

struct program {
    char name[255];
    bool compiled;
    int executedCount;
    int durationMS;
};

int main(int argc, char const **argv){
    int sem_id, shm_id;

    if(argc <2){
        printf("Il manque des arguments ! \n");
        return 0;
    }

    // Sélection du bon type d'action a effectuer
    if(strcmp(argv[1], "1") == 0) {
        printf("action 1 : création des ressources partagées\n");

        //Create semaphore
        sem_id = sem_create(SEMKEY, 1, PERM, 1);
        
        //Create Shared Mermory
        shm_id = sshmget(SHMKEY, sizeof(struct program[1000]), IPC_CREAT | PERM);
        
        printf("La création du répertoire de fichier partagé s'est effectué avec succès ! \n");
    } 
    else if(strcmp(argv[1], "2") == 0) {
        printf("action 2 : destruction des ressources partagées\n");

        sem_id = sem_create(SEMKEY, 1, PERM, 1);
        shm_id = sshmget(SHMKEY, sizeof(struct program[1000]), IPC_CREAT | PERM);
        sshmdelete(shm_id);
        sem_delete(sem_id);
        printf("La suppression du répertoire de fichier partagé s'est effectué avec succès ! \n");
    }
    else if(strcmp(argv[1], "3") == 0){
        if(argc < 3){
            printf("action 3 : il manque un paramètre ! \n");
            return 0;
        }
        printf("action 3 : réservation exclusive des ressources partagées\n");
        
        // get attached memory id
        sem_id = sem_create(SEMKEY, 1, PERM, 1);
        shm_id = sshmget(SHMKEY, sizeof(struct program[1000]) + sizeof(int) , IPC_CREAT | PERM);

        // take access to shared memory
        sem_down0(sem_id);
        // wait for delai of blocking
        sleep(atoi(argv[2]));
        // free the access to the shared memory
        sem_up0(sem_id);
    }
    else { // Default action
        printf("Le type d'action que vous avez inséré est invalide\n");
        return 0;
    }
    return 0;
}
