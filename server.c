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
	if(argc < 2 ){
		printf("Il manque le numéro du port \n");
		exit(0);
	}
	int server_port = atoi(argv[1]);
	int sockfd = initSocketServer(server_port);
	printf("server %i \n",server_port);

	while (true)
	{
		//client trt 
		int newsockfd = saccept(sockfd);

		fork_and_run1(option,&newsockfd);
	}
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
			executeProg(socket, clientmsg.pathLength);
			break;

		//modifier programme existant
		default : 
			printf("modify program\n");
			modifyProgram(clientmsg.name, socket,&clientmsg.num);
			break;
	}
}

void modifyProgram(char* nomFichier, void* sock, void* numProg){
	char readBuffer[BUFFER];
	int* socket = sock;
	int* numProgram = numProg;

	int fd = sopen("./code/newProg.c", O_WRONLY | O_TRUNC | O_CREAT, 0644);

	int nbrRead = sread(*socket,readBuffer, sizeof(readBuffer));
	while (nbrRead != 0){
		nwrite(fd,readBuffer,nbrRead);
		nbrRead = sread(*socket,readBuffer, sizeof(readBuffer));
	}
	sclose(fd);

	/*Acces memoire partagée*/
	int sem_id, shm_id_prog;
	shm_id_prog = sshmget(SHMKEY_PROGRAMMES, sizeof(program[1000]), 0);
	sem_id = sem_get(SEMKEY,1);

	program *programs = sshmat(shm_id_prog);
	int pipe[2];
	spipe(pipe);
	
	int execComp = fork_and_run2(exec_comp,numProgram, &pipe);
	sclose(pipe[1]);
	swaitpid(execComp,NULL,0);

	char outputCompiler[1024];
	serverResponse serverRes;
	nbrRead = 0;
	bool compiled = true;

	while((nbrRead = sread(pipe[0], outputCompiler, sizeof(outputCompiler))) != 0){
		strcpy(serverRes.errorMessage, outputCompiler);
		compiled = false;
	}

	sem_down0(sem_id);
	strcpy(programs[*numProgram].name,nomFichier);
	programs[*numProgram].compiled = compiled;
	programs[*numProgram].executedCount = 0;
	programs[*numProgram].durationMS = 0;
	sem_up0(sem_id);

	sshmdt(programs);
	remove("./code/newProg.c");

	serverRes.num = *numProgram;
	if(compiled){ //program compiled success
		serverRes.compile = 0;
		strcpy(serverRes.errorMessage, "");
	} else { // not compiled
		serverRes.compile = -1;
	}
	swrite(*socket, &serverRes, sizeof(serverRes));
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
	int pipe[2];
	spipe(pipe);
	int c3 = fork_and_run2(exec_comp,indexProg, &pipe);
	sclose(pipe[1]);
	swaitpid(c3,NULL,0);

	char outputCompiler[1024];
	serverResponse serverRes;
	nbrRead = 0;
	bool compiled = true;

	while((nbrRead = sread(pipe[0], outputCompiler, sizeof(outputCompiler))) != 0){
		strcpy(serverRes.errorMessage, outputCompiler);
		compiled = false;
	}
	
	sem_down0(sem_id);
	strcpy(programs[*indexProg].name, nomFichier);
	programs[*indexProg].compiled = compiled;
	programs[*indexProg].durationMS = 0;
	programs[*indexProg].executedCount = 0;
	int fileIndex = *indexProg;
	*indexProg = (*indexProg +1);
	sem_up0(sem_id);

	serverRes.num = fileIndex;
	if(compiled){
		serverRes.compile = 0;
	} else{
		serverRes.compile = -1;
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
	programs = sshmat(shm_id_prog);
	
	//verif prog has compiled
	if(!programs[numProg].compiled){
		serverMessage servermsg;
		servermsg.idProg = numProg;
		servermsg.state = -1;
		servermsg.duration = 0;
		servermsg.exitCode = -1;
		strcpy(servermsg.message, "Le programme n'a pas d'exécutable car il n'a pas réussi a compilé");
		swrite(*socket, &servermsg, sizeof(servermsg));
		return;
	}
	
	char path[15];
	sprintf(path, "./code/%d",numProg);
	struct timeval start, stop;
	int pipeExec[2];

	spipe(pipeExec);

	gettimeofday(&start, NULL);
	int childId = fork_and_run2(exec_exec, path,(void*)&pipeExec);
	sclose(pipeExec[1]);
	swaitpid(childId, NULL, 0);
	gettimeofday(&stop, NULL);

	float duree = ((stop.tv_sec - start.tv_sec) *1000.0f) + ((stop.tv_usec - start.tv_usec) / 1000.0f);

	sem_down0(sem_id);
	programs[numProg].executedCount = (programs[numProg].executedCount)+1;
	programs[numProg].durationMS = (programs[numProg].durationMS) + duree;
	sem_up0(sem_id);
	
	//normal exit
	serverMessage servermsg;
	servermsg.idProg = numProg;
	servermsg.state = 1;
	servermsg.duration = duree;
	servermsg.exitCode = 0;

	int nbrRead;
	char output[1024];
	while((nbrRead = sread(pipeExec[0], output, sizeof(output))) != 0){
		strcpy(servermsg.message, output);
	}
	swrite(*socket, &servermsg, sizeof(serverMessage));
	return;
}

static void exec_comp (void* indexProg, void *pipe){
	int *index = indexProg;
	int *pipeComp = pipe;
	dup2(pipeComp[1], 2);
	sclose(pipeComp[0]);
	sclose(pipeComp[1]);

	char fileName[15];
	sprintf(fileName, "./code/%d", *index);
	sexecl("/usr/bin/gcc","gcc","-o",fileName,"./code/newProg.c",NULL);
	exit(errno);
}

static void exec_exec(void* path,void *pipe){
	char *newPath = path;
	int *pipeExec = pipe;

	dup2(pipeExec[1],1);
	sclose(pipeExec[0]);
	sclose(pipeExec[1]);

	int ret = sexecl(newPath, newPath, NULL);
	exit(ret);
}
