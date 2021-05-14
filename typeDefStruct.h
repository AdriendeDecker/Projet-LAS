#ifndef _SERVERMESSAGE_H_
#define _SERVERMESSAGE_H_

typedef struct SERVERMESSAGE {
    int idProg;
    int state;
    float duration;
    int exitCode;
} serverMessage;

typedef struct PROGRAM {
    char name[255];
    bool compiled;
    int executedCount;
    float durationMS;
} program;

#endif