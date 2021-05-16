#ifndef _SERVERMESSAGE_H_
#define _SERVERMESSAGE_H_

typedef struct SERVERMESSAGE {
    int idProg;
    int state;                      //-2, -1, 0 ou 1
    float duration;
    int exitCode;
    char message[255];
} serverMessage;

typedef struct CLIENTMESSAGE{
    int num;                        //-1 si add, num associé si replace, num associé si exec
    int pathLength;                 //la taille du nom du fichier si add ou replace, -2 si exec
    char name[255];                 //(pour add ou replace)
} clientMessage;

typedef struct SERVERRESPONSE {
    int num;
    int compile;                    //0 si ça compile, autre si pas
    char errorMessage[255];         //msg d'erreur du compilateur
} serverResponse;

typedef struct PROGRAM {
    char name[255];
    bool compiled;
    int executedCount;
    float durationMS;
} program;

#endif