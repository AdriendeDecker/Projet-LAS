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



int main(int argc, char const *argv[]){
    // Vérification inputs Type (et option)
    if(argc <= 1){
        printf("Il manque un type !\n");
        return 0;
    }

    if(!strcmp(argv[1], "3")){
        printf("Le type est 3\n");
        if(argc <= 2){
            printf("Il manque un parametre d'option\n");
            return 0;
        }
    }
    // Fin de vérifications inputs

    // Sélection du bon type d'action a effectuer
    if(strcmp(argv[1], "1") == 0){
        printf("action 1 : création des ressources partagées\n");

        //Create semaphore
        int sem_id = sem_create(SEMKEY, 1, PERM, 1);
        printf("mon sem_id = %d\n",sem_id);
        
        //Create Shared Mermory
        int shm_id = sshmget(SHMKEY, sizeof(struct program[1000]) + sizeof(int) , IPC_CREAT | PERM);
        printf("mon shm_id = %d\n", shm_id);
    } 
    else if(strcmp(argv[1], "2") == 0){
        printf("action 2 : destruction des ressources partagées\n");
    }
    else if(strcmp(argv[1], "3") == 0){
        printf("action 3 : réservation exclusive des ressources partagées\n");
    }
    else { // Default action
        printf("Le type d'action que vous avez inséré est invalide\n");
        return 0;
    }

    return 0;
}
