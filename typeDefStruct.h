#ifndef _SERVERMESSAGE_H_
#define _SERVERMESSAGE_H_

typedef struct SERVERMESSAGE {
    int idProg;
    int state;
    float duration;
    int exitCode;
} serverMessage;

typedef struct CLIENTMESSAGE{
    int num;                        //-1 si add, num associé si replace, num associé si exec
    int pathLength;                 //la taille du nom du fichier si add ou replace, -2 si exec
    char name[255];                 //(pour add ou replace)
} clientMessage;

typedef struct SERVERRESPONSE {
    int num;
    int compile;                    //(pour add ou replace) 0 si ça compile, autre si pas
    char errorMessage[255];         //(pour add ou replace) msg d'erreur du compilateur
    int state;                      //(pour exec) -2, -1, 0 ou 1
    int executionTime;              //(pour exec)
    int exitCode;                   //(pour exec)
} serverResponse;

typedef struct PROGRAM {
    char name[255];
    bool compiled;
    int executedCount;
    float durationMS;
} program;

#endif