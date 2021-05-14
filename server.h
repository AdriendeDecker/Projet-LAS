#ifndef _SERVER_H_
#define _SERVER_H_

int main(int argc, char *argv[]);

static void option(void *arg);

void executeProg(void *arg);

void addProgram(char* nomFichier, void* sock);

void readBlock();

static void exec_comp (void *arg,void* indexProg);

static void exec_cat(void *arg);

static void exec_ls(void *arg);

int initSocketServer(int port);

#endif