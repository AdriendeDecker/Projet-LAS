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

void readBlock(){
	size = sread(0,buffer,BUFFER);	
}


void addOrChange (int nbrCar,char* nomFichier, char* contenu){

	int fd;

	fd = sopen(nomFichier, O_WRONLY | O_TRUNC | O_CREAT, 0644);

	swrite(fd,contenu,nbrCar);
	

}

int main(int argc, char *argv[])
{
	readBlock();
	addOrChange(size,argv[1],buffer);
	/* 
	int sockfd;

	sockfd = initSocketServer(SERVER_PORT);
	printf("server %i \n",SERVER_PORT);

	client trt 
	int newsockfd = accept(sockfd,NULL,NULL);
	 */
}


