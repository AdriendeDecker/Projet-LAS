#ifndef _SERVERMESSAGE_H_
#define _SERVERMESSAGE_H_

typedef struct SERVERMESSAGE {
    int idProg;
    int state;
    float duration;
    int exitCode;
} serverMessage;

typedef struct {
    int num;
    int pathLength;
    char name[255];
} clientMessage;

#endif