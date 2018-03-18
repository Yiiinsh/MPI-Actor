#ifndef __ACTOR_MESSAGE_H
#define __ACTOR_MESSAGE_H

typedef enum _message_type {
     CREATE,
     EXEC,
     TERMINATE
} MESSAGE_TYPE;

typedef struct {
    MESSAGE_TYPE msg_type;
    char *args;
    int src;
    int dest;
} actor_message;

#endif