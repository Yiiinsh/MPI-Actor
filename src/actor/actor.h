#ifndef __ACTOR_H
#define __ACTOR_H

#include <stdbool.h>

#include "accotr_message.h"

#define TYPE_LIMIT 128

/* Abstract Actor */
typedef struct _actor {
    /* Attributes */
    char type[TYPE_LIMIT]; // actor type
    int attr_size; // size of attribute
    void **attrs; // attributes of this actor
    bool loop_flag; // main loop condition

    /* Actor Behaviour Function */
    void (*on_message)(actor_message msg);
    void (*send_message)(actor_message msg, int dest);
    void (*execute_step)(int argc, void **argv);
    actor* (*new_actor)(char *type, int count);

    /* Actor Process Function */
    void (*pre_process)(int argc, void **argv);
    void (*post_process)(int argc, void **argv);
    void (*terminate)();
} actor;

#endif