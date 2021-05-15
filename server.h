#ifndef _SERVER_H_
#define _SERVER_H_

int main(int argc, char *argv[]);

static void option(void *arg);

void executeProg(void *arg, int numProg);

void addProgram(char* nomFichier, void* sock);

void modifyProgram(char* nomFichier, void* sock, void* numProg);

void readBlock();

static void exec_comp(void* indexProg);

static void exec_cat(void *arg);

int initSocketServer(int port);

#endif