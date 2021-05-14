#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "utils_v10.h"
#include "serverMessage.h"

#define COMMAND_SIZE 255
#define MAX_PROGRAMMES 100

int initSocket(char* adr, int port);
void applyStrtoken (char* buffer, char** save);
void add(char* adr, int port, char* path);
void replace(char* adr, int port, int num, char* path);
void execute1(char* adr, int port, int num);

//child minuteur
void minuteurProcess(void *arg1, void *arg2) {
    int *delay = arg1;
    int *pipefd = arg2;
    int battement = -1;

    //pipe écriture
    sclose(pipefd[0]);

    //condition à trouver (via un quit ou autre par exemple)
    while(1) {
        sleep(*delay);
        swrite(pipefd[1],&battement, sizeof(int));
    }

    sclose(pipefd[1]);
}

//child d'exécution récurrente
void execProcess(void *arg1, void *arg2, void *arg3) {
    int *port = arg1;
    char *adr = arg2;
    int *pipefd = arg3;
    int nbrProgrammes = 0;
    int tabProgrammes[MAX_PROGRAMMES];
    int numProgramme;

    //pipe lecture
    sclose(pipefd[1]);

    int nbInt = read(pipefd[0], &numProgramme, sizeof(int));
    while(nbInt > 0) {
        tabProgrammes[nbrProgrammes] = numProgramme;
        nbrProgrammes++;
    }

    sclose(pipefd[0]);
}

int main(int argc, char** argv) {

    int pipefd[2];
    char bufferCmd[COMMAND_SIZE];
    char* save[3];
    bool validCmd = false;
    bool stop = false;

    int port = atoi(argv[2]);
    int delay = atoi(argv[3]);
    char* adr = argv[1];

    spipe(pipefd);
    int childExec = fork_and_run3(execProcess, &port, &adr, pipefd);
    int childMinuteur = fork_and_run2(minuteurProcess, &delay, pipefd);
    //pipe écriture
    sclose(pipefd[0]);
    
    while(!stop) {
        printf("Veuillez introduire votre commande (+,.,*,@) suivie de la donnée requise : \n");
        applyStrtoken(bufferCmd, save);
        char* path;
        int num;
        while(!validCmd){
            if(save[0] == "+") {
                path = save[1];
                add(adr, port, path);
                validCmd = true;
            } else if(save[0] == ".") {
                num = atoi(save[1]);
                path = save[2];
                replace(adr, port, num, path);
                validCmd = true;
            } else if(save[0] == "@") {
                num = atoi(save[1]);
                execute1(adr, port, num);
                validCmd = true;
            } else if(save[0] == "*") {
                num = atoi(save[1]);
                //voir avec fils
                validCmd = true;
            } else if(save[0] == "q") {
                stop = true;
                //free la liste des programmes (via un signal pour le minuteur et via la fermeture du pipe pour les programmes récurrents)
            } else {
                printf("Commande invalide, veuillez réintroduire votre commande (+,.,*,@) suivie de la donnée requise : \n");
                applyStrtoken(bufferCmd, save);
            }
        }
        for(int i = 0; i<3; i++) {
            free(save[i]);
        }
    }

    sclose(pipefd[1]);

    exit(0);
}


//ajout d'un programme
void add(char* adr, int port, char* path) {
    replace(adr, port, -1, path);
}

//remplacement d'un programme
void replace(char* adr, int port, int num, char* path) {
    int sockfd = initSocket(adr, port);
    int filefd = sopen(path, O_RDONLY, 0700);
    int pathlength = strlen(path);

    clientMessage msg;
    msg.num = num;
    msg.pathLength = pathlength;
    strcpy(msg.name, path);

    swrite(sockfd, &msg, sizeof(clientMessage));
}

//exécution d'un seul programme
void execute1(char* adr, int port, int num) {

}

//initialise le socket
int initSocket(char* adr, int port) {
    int sockfd = ssocket();
    sconnect(adr, port, sockfd);
    return sockfd;
}

//utilisée pour séparer les différentes données de la ligne rentrée par le client
void applyStrtoken (char* buffer, char** save) {
    sread(0, buffer, COMMAND_SIZE);
    char* separator = " ";
    char* strToken = strtok(buffer, separator);
    int i = 0;
    while (strToken != NULL) {
        save[i] = strToken;
        i++;
        strToken = strtok(NULL, separator);
    }   
}
