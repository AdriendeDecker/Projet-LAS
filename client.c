#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "utils_v10.h"
#include "typeDefStruct.h"

#define MAX_CMD_SIZE 255
#define MAX_PROGRAMMES 100

volatile sig_atomic_t end_recurr = 0;

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

    
    while(end_recurr == 0) {                                                         //tant que le signal de stop n'est pas envoyé
        sleep(*delay);
        swrite(pipefd[1],&battement, sizeof(int));
    }

    sclose(pipefd[1]);
}

//signal handler pour stopper le minuteur
void stop_timer_handler() {
    end_recurr = 1;
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

    int nbInt = sread(pipefd[0], &numProgramme, sizeof(int));                       //lit le pipe et stocke les nums des programmes (si c'est -1, c'est un battement)
    while(nbInt > 0) {
        if(numProgramme >= 0) {                                                      //si on lit un programme
            tabProgrammes[nbrProgrammes] = numProgramme;                            //ajoute le programme à la liste
            nbrProgrammes++;
        }else {                                                                     //si c'est un battement de coeur
            for(int i=0; i<nbrProgrammes; i++) {
                execute1(adr, *port, tabProgrammes[i]);                             //exécute chaque programme du tableau
            }
        }
        nbInt = sread(pipefd[0], &numProgramme, sizeof(int));
    }

    sclose(pipefd[0]);
}

int main(int argc, char** argv) {

    int pipefd[2];
    char bufferCmd[MAX_CMD_SIZE];
    char* save[3];
    bool validCmd = false;
    bool stop = false;

    int port = atoi(argv[2]);
    int delay = atoi(argv[3]);
    char* adr = argv[1];

    ssigaction(SIGUSR1, stop_timer_handler);
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
            if(strcmp(save[0], "+") == 0) {
                path = save[1];
                add(adr, port, path);
                validCmd = true;
            } else if(strcmp(save[0], ".") == 0) {
                num = atoi(save[1]);
                path = save[2];
                replace(adr, port, num, path);
                validCmd = true;
            } else if(strcmp(save[0], "@") == 0) {
                num = atoi(save[1]);
                execute1(adr, port, num);
                validCmd = true;
            } else if(strcmp(save[0], "*") == 0) {
                num = atoi(save[1]);
                swrite(pipefd[1], &num, sizeof(int));                       //permet de récupérer les nums dans le child d'exec
                validCmd = true;
            } else if(strcmp(save[0], "q") == 0) {
                validCmd = true;
                stop = true;
                //free la liste des programmes (via un signal pour le minuteur et via la fermeture du pipe pour les programmes récurrents)
            } else {
                printf("Commande invalide, veuillez réintroduire votre commande (+,.,*,@) suivie de la donnée requise : \n");
                applyStrtoken(bufferCmd, save);
            }
        }
        validCmd = false;
    }

    skill(childMinuteur, SIGUSR1);

    sclose(pipefd[1]);

    exit(0);
}


//ajout d'un programme
void add(char* adr, int port, char* path) {
    int sockfd = initSocket(adr, port);
    int filefd = sopen(path, O_RDONLY , 0400);
    int pathlength = strlen(path);

    clientMessage msg;
    msg.num = -1;
    msg.pathLength = pathlength;
    strcpy(msg.name, path);

    swrite(sockfd, &msg, sizeof(clientMessage));

    printf("%s\n", path);
    char buffer[BUFFER_SIZE];
    int nbCharRd = sread(filefd, buffer, sizeof(buffer));
    while(nbCharRd != 0) {
        swrite(sockfd, buffer, nbCharRd);
        nbCharRd = sread(filefd, buffer, sizeof(buffer));
    }
    shutdown(sockfd, SHUT_WR);

    serverResponse response;
    sread(sockfd, &response, sizeof(serverResponse));
    printf("Programme num %d ajouté \nCode de compilation : %d \nMessage d'erreur : %s \n", 
        response.num, response.compile, response.errorMessage);
    sclose(sockfd);
}

//remplacement d'un programme
void replace(char* adr, int port, int num, char* path) {
    int sockfd = initSocket(adr, port);
    int filefd = sopen(path, O_RDONLY, 0700);                                   //ouvre le fichier à remplacer
    int pathlength = strlen(path);

    clientMessage msg;                                                          //p4 point 2.a
    msg.num = num;
    msg.pathLength = pathlength;
    strcpy(msg.name, path);
    
    swrite(sockfd, &msg, sizeof(clientMessage));                                //écrit ce qui est stocké dans msg dans sockfd

    char buffer[BUFFER_SIZE];                                                   //va copier ce qui se trouve dans le fichier et le mettre dans le socket
    int nbCharRd = sread(filefd, buffer, sizeof(buffer));
    while(nbCharRd != 0) {
        swrite(sockfd, buffer, nbCharRd);
        nbCharRd = sread(filefd, buffer, sizeof(buffer));
    }
    shutdown(sockfd, SHUT_WR);                                                //bloque la connexion en write (close la supprime)

    serverResponse response;
    sread(sockfd, &response, sizeof(serverResponse));                           //récupère les infos dans le socket et les mets dans response
    printf("Programme num %d remplacé \nCode de compilation : %d \nMessage d'erreur : %s \n", 
        response.num, response.compile, response.errorMessage);
    sclose(sockfd);
}

//exécution d'un seul programme
void execute1(char* adr, int port, int num) {
    int sockfd = initSocket(adr, port);

    clientMessage msg;
    msg.num = -2;                                   //demandé de return -2
    msg.pathLength = num;                           //num du program a executer
    strcpy(msg.name, "");                           //rien demandé ici

    swrite(sockfd, &msg, sizeof(clientMessage));

    serverMessage response;
    sread(sockfd, &response, sizeof(serverMessage));                //read les données récupérées du serveur et les stocke dans la response
    printf("Programme num %d exécuté \nEtat du programme : %d \nTemps d'exécution : %f \nCode de retour : %d \n", 
        response.idProg, response.state, response.duration, response.exitCode);
    sclose(sockfd);
}

//initialise le socket
int initSocket(char* adr, int port) {
    int sockfd = ssocket();
    sconnect(adr, port, sockfd);
    return sockfd;
}

//utilisée pour séparer les différentes données de la ligne rentrée par le client
void applyStrtoken (char* buffer, char** save) {
    sread(0, buffer, MAX_CMD_SIZE);
    char* separator = " \n";
    char* strToken = strtok(buffer, separator);
    int i = 0;
    while (strToken != NULL) {
        save[i] = strToken;
        i++;
        strToken = strtok(NULL, separator);
    }   
}
