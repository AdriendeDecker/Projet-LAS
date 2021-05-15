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
#define SERVER_PORT 2500
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

// PRE:  ServerPort: a valid port number
// POST: on success bind a socket to 0.0.0.0:port and listen to it
//       return socket file descriptor
//       on failure, displays error cause and quits the program
int initSocketServer(int port){
	int sockfd = ssocket();

	sbind(port, sockfd);

	slisten(sockfd, BACKLOG);

	//int option = 1;
	// setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int));

	return sockfd;
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

	int execComp = fork_and_run2(exec_comp,nomFichier,numProgram);
	swaitpid(execComp,NULL,0);

	sem_down0(sem_id);
	*programs[*numProgram].name = *nomFichier;
	programs[*numProgram].compiled = (codeExec!=-1);
	programs[*numProgram].executedCount = 0;
	programs[*numProgram].durationMS = 0;
	sem_up0(sem_id);

	sshmdt(programs);
	remove(nomFichier);
	codeExec = 0;
}

void addProgram (char* nomFichier, void* sock){
	char readBuffer[BUFFER];
	int *socket = sock;
	
	int fd = sopen(nomFichier, O_WRONLY | O_TRUNC | O_CREAT, 0744);

	int nbrRead = sread(*socket,readBuffer, BUFFER);
	while (nbrRead != 0){
		nwrite(fd,readBuffer,nbrRead);
		nbrRead = sread(*socket,readBuffer,BUFFER);
	}
	sclose(fd);

	int c2 = fork_and_run1(exec_cat,nomFichier);
	swaitpid(c2,NULL,0);
	
	/*Acces memoire partagée*/
	int sem_id, shm_id_prog, shm_id_index;
	shm_id_prog = sshmget(SHMKEY_PROGRAMMES, sizeof(program[1000]), 0);
	shm_id_index = sshmget(SHMKEY_INDEX, sizeof(int), 0);
	sem_id = sem_get(SEMKEY,1);

	program *programs = sshmat(shm_id_prog);
	int *indexProg = sshmat(shm_id_index);

	/*Compiler */
	int c3 = fork_and_run2(exec_comp,nomFichier,indexProg);
	swaitpid(c3,NULL,0);

	sem_down0(sem_id);
	*programs[*indexProg].name = *nomFichier;
	programs[*indexProg].compiled = (codeExec!=-1);
	programs[*indexProg].durationMS = 0;
	programs[*indexProg].executedCount = 0;

	if(codeExec == 0){
		/*Execution*/
		printf("./%d:\n",*indexProg);
		char prognum[5];
		sprintf(prognum,"./%d",*indexProg);
		sexecl(prognum,NULL);
	}

	*indexProg = (*indexProg +1);
	sem_up0(sem_id);

	sshmdt(indexProg);
	sshmdt(programs);
	remove(nomFichier);
	codeExec =0;
}

void executeProg(void* sock){
	int *socket = sock;
	int sem_id, shm_id_prog, shm_id_index;
	shm_id_prog = sshmget(SHMKEY_PROGRAMMES, sizeof(program[1000]), 0);
	shm_id_index = sshmget(SHMKEY_INDEX, sizeof(int), 0);
	sem_id = sem_get(SEMKEY,1);

	program* programs;
	programs = sshmat(shm_id_prog);
	int *indexProg = sshmat(shm_id_index);

	char* nextarg = strtok(NULL, limiter);
	int idProg = atoi(nextarg);
	
	//verif prog exist
	if(idProg>=*indexProg){
		serverMessage servermsg;
		servermsg.idProg = idProg;
		servermsg.state = -2;
		servermsg.duration = 0;
		servermsg.exitCode = -1;
		swrite(*socket, &servermsg, sizeof(servermsg));
		return;
	}
	
	//verif prog has compiled
	if(!programs[idProg].compiled){
		serverMessage servermsg;
		servermsg.idProg = idProg;
		servermsg.state = -1;
		servermsg.duration = 0;
		servermsg.exitCode = -1;
		swrite(*socket, &servermsg, sizeof(servermsg));
		return;
	}
	char* path = "./";
	strcat(path, nextarg);

	struct timeval start, stop;

	gettimeofday(&start, NULL);
	int exitCode = sexecl(path,NULL);
	gettimeofday(&stop, NULL);

	float duree = ((stop.tv_sec - start.tv_sec) *1000.0f) + ((stop.tv_usec - start.tv_usec) / 1000.0f);

	sem_down0(sem_id);
	programs[idProg].executedCount = (programs[idProg].executedCount)+1;
	programs[idProg].durationMS = (programs[idProg].durationMS) + duree;
	sem_up0(sem_id);

	//error during execution
	if(exitCode == -1){
		serverMessage servermsg;
		servermsg.idProg = idProg;
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
	servermsg.idProg = idProg;
	servermsg.state = 1;
	servermsg.duration = duree;
	servermsg.exitCode = 0;
	swrite(*socket, &servermsg, sizeof(servermsg));
	//send stdout
	//TODO
	return;
}

/*Choix de l'option*/
static void option(void *arg){
	int *socket = arg;
	char input[BUFFER];
	sread (*socket,&input,sizeof(input));
	
	char* token;
	//token = premier arg recu
	token = strtok(input, limiter);
	int operation = atoi(token);

	switch(operation){
		//ajouter nouveau programme
		case -1:
			printf("add program\n");
			char* nomFichier = strtok(NULL, limiter);
			addProgram(nomFichier,&socket);
			break;
		
		//exécuter programme existant
		case -2:
			printf("execute program\n");
			executeProg(&socket);
			break;

		//modifier programme existant
		default : 
			printf("modify program\n");
			char* nomFile = strtok(NULL, limiter);
			modifyProgram(nomFile, &socket,&operation);
			break;
	}
}


static void exec_cat(void *arg){
	char *scriptname = arg;
	printf("cat %s : \n",scriptname);
	sexecl("/bin/cat","cat",scriptname,NULL);
}

static void exec_comp (void *arg,void* indexProg){
	char *scriptName = arg;
	int *index = indexProg;
	codeExec = sexecl("/usr/bin/gcc","gcc","-o",index,scriptName,NULL);
}

void readBlock(){
	size = sread(0,buffer,BUFFER);	
}
