#ifndef _SERVERMESSAGE_H_
#define _SERVERMESSAGE_H_

typedef struct SERVERMESSAGE {
    int idProg;
    int state;
    float duration;
    int exitCode;
} serverMessage;

#endif