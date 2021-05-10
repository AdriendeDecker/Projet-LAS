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

#define BACKLOG 50
#define SERVER_PORT 2500
#define BUFFER 255
#define MAX_CONNECTION 50
#define SHMKEY_PROGRAMMES 123
#define SHMKEY_INDEX 456
#define SEMKEY 789

int idx = -1;
int size = -1;
char buffer[BUFFER];

typedef struct PROGRAM {
    char name[255];
    bool compiled;
    int executedCount;
    int durationMS;
} program;


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



static void exec_ls(void *arg){
	char *scriptName = arg;
	printf("-ls -l %s:\n", scriptName);
	sexecl("/bin/ls","ls","-ls",scriptName,NULL);
}

static void exec_cat(void *arg){
	char *scriptname = arg;
	printf("cat %s : \n",scriptname);
	sexecl("/bin/cat","cat",scriptname,NULL);
}

static void exec_comp (void *arg){
	char *scriptName = arg;
	sexecl("/usr/bin/gcc","gcc","-o","2",scriptName,NULL);
}

void readBlock(){
	size = sread(0,buffer,BUFFER);	
}


void addOrChange (int nbrCar,char* nomFichier, char* contenu){

	int fd;

	fd = sopen(nomFichier, O_WRONLY | O_TRUNC | O_CREAT, 0744);

	/* Faire un ls -l sur le nom du fichier créé*/
	int c1 = fork_and_run1(exec_ls,nomFichier);
	swaitpid(c1,NULL,0);

	/*ecriture du contenu  */
	nwrite(fd,contenu,nbrCar);

	/*Liberation ressource*/
	sclose(fd);

	int c2 = fork_and_run1(exec_cat,nomFichier);
	swaitpid(c2,NULL,0);

	/*Compiler */
	int c3 = fork_and_run1(exec_comp,nomFichier);
	swaitpid(c3,NULL,0);

	/*Execution*/
	printf("./%s:\n","2");
	sexecl("./2",NULL);
}

/*Choix de l'option*/
static void option(void *arg){
	int *socket = arg;
	char input[BUFFER];
	sread (*socket,&input,sizeof(input));
	const char limiter[1]= " ";
	char* token;
	//token = premier arg recu
	token = strtok(input, limiter);
	int operation = atoi(token);
	int sem_id, shm_id_prog, shm_id_index;


	switch(operation){
		//ajouter nouveau programme
		case -1:
			printf("Ajout d'un programme\n");
			char* nomFichier = strtok(NULL, limiter);
			
			break;
		
		//exécuter programme existant
		case -2:
			printf("-2\n");
			shm_id_prog = sshmget(SHMKEY_PROGRAMMES, sizeof(program[1000]), 0);
        	shm_id_index = sshmget(SHMKEY_INDEX, sizeof(int), 0);

			program* programs;
			int* programIndex;

			programs = sshmat(shm_id_prog);
			programIndex = sshmat(shm_id_index);

			char* nextarg = strtok(NULL, limiter);
			int idProg = atoi(nextarg);
			char* path = "./";
			strcat(&path, nextarg);

			struct timeval start, stop;
			gettimeofday(&start, NULL);
			sexecl(path,NULL);
			gettimeofday(&stop, NULL);

			float duree = ((stop.tv_sec - start.tv_sec) *1000.0f) + ((stop.tv_usec - start.tv_usec) / 1000.0f);

			sem_down0(sem_id);
			programs[idProg].executedCount = programs[idProg].executedCount++;
			sem_up0(sem_id);
			
			break;

		//modifier programme existant
		default : 
			printf("num programme\n");
			break;
	}

}

int main(int argc, char *argv[])
{
 
	int sockfd = initSocketServer(SERVER_PORT);
	printf("server %i \n",SERVER_PORT);

	while (true)
	{
		//client trt 
		int newsockfd = saccept(sockfd);


		fork_and_run1(option,&newsockfd);
	
	}
	
	


	readBlock();
	addOrChange(size,argv[1],buffer);
	
}


