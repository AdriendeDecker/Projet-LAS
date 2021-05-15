#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#include "utils_v10.h"
#include "server.h"
#include "typeDefStruct.h"

#define BACKLOG 50
#define SERVER_PORT 6003
#define BUFFER 256
#define MAX_CONNECTION 50
#define SHMKEY_PROGRAMMES 123
#define SHMKEY_INDEX 456
#define SEMKEY 789

int idx = -1;
int size = -1;
char buffer[BUFFER];
static volatile int codeExec = 0;
const char limiter[1]= " ";

int main(int argc, char *argv[]) {
	//port server doit venir de argv => ex: ./server 9090
	int sockfd = initSocketServer(SERVER_PORT);
	printf("server %i \n",SERVER_PORT);

	while (true)
	{
		//client trt 
		int newsockfd = saccept(sockfd);

		fork_and_run1(option,&newsockfd);
	}
	readBlock();
}

int initSocketServer(int port){
	int sockfd = ssocket();

	sbind(port, sockfd);

	slisten(sockfd, BACKLOG);

	return sockfd;
} 

/*Choix de l'option*/
static void option(void *arg){
	int *socket = arg;
	clientMessage clientmsg;
	sread (*socket,&clientmsg,sizeof(clientmsg));

	switch(clientmsg.num){
		//ajouter nouveau programme
		case -1:
			printf("add program\n");
			addProgram(clientmsg.name,socket);
			break;
		
		//exécuter programme existant
		case -2:
			printf("execute program\n");
			executeProg(&socket, clientmsg.pathLength);
			break;

		//modifier programme existant
		default : 
			printf("modify program\n");
			modifyProgram(clientmsg.name, &socket,&clientmsg.num);
			break;
	}
}

void modifyProgram(char* nomFichier, void* sock, void* numProg){
	char readBuffer[BUFFER];
	int* socket = sock;
	int* numProgram = numProg;

	int fd = sopen(nomFichier, O_WRONLY | O_TRUNC | O_CREAT, 0744);

	int nbrRead = sread(*socket, readBuffer, BUFFER);
	while(nbrRead != 0){
		swrite(fd, readBuffer, nbrRead);
		nbrRead = sread(*socket,readBuffer,BUFFER);
	}
	sclose(fd);

	int c2 = fork_and_run1(exec_cat,nomFichier);
	swaitpid(c2,NULL,0);

	/*Acces memoire partagée*/
	int sem_id, shm_id_prog;
	shm_id_prog = sshmget(SHMKEY_PROGRAMMES, sizeof(program[1000]), 0);
	sem_id = sem_get(SEMKEY,1);

	program *programs = sshmat(shm_id_prog);

	int execComp = fork_and_run1(exec_comp,numProgram);
	swaitpid(execComp,NULL,0);

	sem_down0(sem_id);
	*programs[*numProgram].name = *nomFichier;
	programs[*numProgram].compiled = (codeExec!=-1);
	programs[*numProgram].executedCount = 0;
	programs[*numProgram].durationMS = 0;
	sem_up0(sem_id);

	sshmdt(programs);
	remove(nomFichier);

	char retour[25];
	sprintf(retour, "%d %d", *numProgram, execComp);
	swrite(*socket, retour, sizeof(retour));
	//rajouter msg compilateur si pas réussi a compiler

	codeExec = 0;
}

void addProgram (char* nomFichier, void* sock){
	char readBuffer[BUFFER];
	int *socket = sock;

	int fd = sopen("./code/newProg.c", O_WRONLY | O_TRUNC | O_CREAT, 0644);
	int nbrRead = sread(*socket,readBuffer, sizeof(readBuffer));
	while (nbrRead != 0){
		nwrite(fd,readBuffer,nbrRead);
		nbrRead = sread(*socket,readBuffer, sizeof(readBuffer));
	}
	sclose(fd);

	/*Acces memoire partagée*/
	int sem_id, shm_id_prog, shm_id_index;
	shm_id_prog = sshmget(SHMKEY_PROGRAMMES, sizeof(program[1000]), 0);
	shm_id_index = sshmget(SHMKEY_INDEX, sizeof(int), 0);
	sem_id = sem_get(SEMKEY,1);

	program *programs = sshmat(shm_id_prog);
	int *indexProg = sshmat(shm_id_index);

	/*Compiler */
	int c3 = fork_and_run1(exec_comp,indexProg);
	swaitpid(c3,NULL,0);

	sem_down0(sem_id);
	strcpy(programs[*indexProg].name, nomFichier);
	programs[*indexProg].compiled = (codeExec!=-1);
	programs[*indexProg].durationMS = 0;
	programs[*indexProg].executedCount = 0;
	int fileIndex = *indexProg;
	*indexProg = (*indexProg +1);
	sem_up0(sem_id);
	printf("la sémaphore a été libérée\n");

	serverResponse serverRes;
	serverRes.num = fileIndex;
	printf("%d\n", fileIndex);
	if(codeExec != -1){ //program compiled success
		serverRes.compile = 0;
	} else { // not compiled
		serverRes.compile = -1;
		//TODO
		strcpy(serverRes.errorMessage, "mon message d'erreur TODO");
	}
	swrite(*socket, &serverRes, sizeof(serverRes));

	sshmdt(indexProg);
	sshmdt(programs);
	remove("./code/newProg.c");
	codeExec = 0;
}

void executeProg(void* sock, int numProg){
	int *socket = sock;
	int sem_id, shm_id_prog;
	shm_id_prog = sshmget(SHMKEY_PROGRAMMES, sizeof(program[1000]), 0);
	sem_id = sem_get(SEMKEY,1);

	program* programs;
	printf("avant verif 1 ?");
	programs = sshmat(shm_id_prog);
	printf("avant verif 2 ?");
	
	//verif prog has compiled
	if(!programs[numProg].compiled){
		serverMessage servermsg;
		servermsg.idProg = numProg;
		servermsg.state = -1;
		servermsg.duration = 0;
		servermsg.exitCode = -1;
		swrite(*socket, &servermsg, sizeof(servermsg));
		return;
	}

	char path[15] = "./code/";
	char num[4];
	sprintf(num, "%d",numProg);
	strcat(path, num);
	
	struct timeval start, stop;

	gettimeofday(&start, NULL);
	int exitCode = sexecl(path,NULL);
	gettimeofday(&stop, NULL);

	float duree = ((stop.tv_sec - start.tv_sec) *1000.0f) + ((stop.tv_usec - start.tv_usec) / 1000.0f);

	sem_down0(sem_id);
	programs[numProg].executedCount = (programs[numProg].executedCount)+1;
	programs[numProg].durationMS = (programs[numProg].durationMS) + duree;
	sem_up0(sem_id);

	printf("on arrive ici ?");

	//error during execution
	if(exitCode == -1){
		serverMessage servermsg;
		servermsg.idProg = numProg;
		servermsg.state = 0;
		servermsg.duration = 0;
		servermsg.exitCode = exitCode;
		swrite(*socket, &servermsg, sizeof(servermsg));
		//send stdout
		//TODO
		return;
	}

	printf("duree prog = %f", duree);
	
	//normal exit
	serverMessage servermsg;
	servermsg.idProg = numProg;
	servermsg.state = 1;
	servermsg.duration = duree;
	servermsg.exitCode = 0;
	swrite(*socket, &servermsg, sizeof(servermsg));
	//send stdout
	//TODO
	return;
}

static void exec_cat(void *arg){
	char *scriptname = arg;
	printf("cat %s : \n",scriptname);
	sexecl("/bin/cat","cat",scriptname,NULL);
}

static void exec_comp (void* indexProg){
	printf("debut exec comp");
	int *index = indexProg;
	char fileName[15];
	sprintf(fileName, "./code/%d", *index);
	codeExec =sexecl("/usr/bin/gcc","gcc","-o",fileName,"./code/newProg.c",NULL);
	printf("fin exec comp");
}

void readBlock(){
	size = sread(0,buffer,BUFFER);	
}
