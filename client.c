#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "utils_v10.h"

#define COMMAND_SIZE 255

/*void minuteurProcess(void *arg1, void *arg2) {
    int *delay = arg1;
    int *pipefd = arg2;

    sclose(pipefd[0]);
    //pipe écriture

    //condition à trouver (via un quit ou autre par exemple)
    while(1) {
        sleep(*delay);
    }

    sclose(pipefd[1]);
}

void execProcess(void *arg) {
    int *pipefd = arg;

    sclose(pipefd[1]);
    //pipe lecture

    sclose(pipefd[0]);
}*/

int main(int argc, char** argv) {
    /*int pipefd[2];
    int delay = argv[3];

    spipe(pipefd);
    int childMinuteur = fork_and_run2(minuteurProcess, delay, pipefd);
    int childExec = fork_and_run1(execProcess, pipefd);
    sclose(pipefd[0]);
    //pipe écriture

    //mettre un wait dans une boucle (max 100 ou jusqu'au bout des programmes) qui attend le minuteur avant d'exécuter la commande suivante ?
    int i = 0;
    while(tous programmes pas finis || i < 100) {
       
    }

    sclose(pipefd[1]);*/

    char bufferCmd[COMMAND_SIZE];
    bool validCmd = false;

    printf("Veuillez introduire votre commande (+,.,*,@) suivie de la donnée requise : \n");
    int nbChar = read(0, bufferCmd, COMMAND_SIZE);
    while(!validCmd){
        if(bufferCmd[0] == "+") {
            char bufferAdd[BUFFER_SIZE];
            validCmd = true;
        } else if(bufferCmd[0] == ".") {
            validCmd = true;
        } else if(bufferCmd[0] == "@") {
            validCmd = true;
        } else if(bufferCmd[0] == "*") {
            validCmd = true;
        } else if(bufferCmd[0] == "q") {
            //free la liste des programmes (via un signal pour le minuteur et via la fermeture du pipe pour les programmes récurrents)
            exit(0);
        } else {
            printf("Commande invalide, veuillez réintroduire votre commande (+,.,*,@) suivie de la donnée requise : \n");
            nbChar = read(0, bufferCmd, COMMAND_SIZE);
        }
    }
}