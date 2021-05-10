#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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
//#define SERVER_PORT 900
#define BUFFER 255


// PRE:  ServerPort: a valid port number
// POST: on success bind a socket to 0.0.0.0:port and listen to it
//       return socket file descriptor
//       on failure, displays error cause and quits the program

/* int initSocketServer(int port){
	int sockfd = ssocket();

	sbind(port, sockfd);

	slisten(sockfd, BACKLOG);

	int option = 1;
	// setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int));

	return sockfd;
} */

int idx = -1;
int size = -1;
char buffer[BUFFER];

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

void readBlock(){
	size = sread(0,buffer,BUFFER);	
}


void addOrChange (int nbrCar,char* nomFichier, char* contenu){

	int fd;

	fd = sopen(nomFichier, O_WRONLY | O_TRUNC | O_CREAT, 0644);

	/* Faire un ls -l sur le nom du fichier créé*/
	int c1 = fork_and_run1(exec_ls,nomFichier);
	swaitpid(c1,NULL,0);

	/*Ecriture du shebang*/
	char* shebang = "!/bin/bash\n";
	int szShebang = strlen(shebang);
	nwrite(fd,shebang,szShebang);

	/*ecriture du contenu  */
	nwrite(fd,contenu,nbrCar);

	/*Liberation ressource*/
	sclose(fd);

	int c2 = fork_and_run1(exec_cat,nomFichier);
	swaitpid(c2,NULL,0);

	/*Execution*/
	printf("./%s:\n",nomFichier);
	sexecl(nomFichier,nomFichier,NULL);
}

int main(int argc, char *argv[])
{

	readBlock();
	addOrChange(size,argv[1],buffer);
	//sexecl("/bin/echo",argv[1]);
	/* 
	int sockfd;

	sockfd = initSocketServer(SERVER_PORT);
	printf("server %i \n",SERVER_PORT);

	client trt 
	int newsockfd = accept(sockfd,NULL,NULL);
	 */
}


