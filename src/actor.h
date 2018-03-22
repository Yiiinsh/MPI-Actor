#ifndef __ACTOR_H
#define __ACTOR_H

#include <stdbool.h>
#include <mpi.h>

#define ACTOR_TYPE_NAME_LIMIT 64
#define ATTR_SIZE_LIMIT 10

typedef struct __actor
{
    char type[ACTOR_TYPE_NAME_LIMIT];
    bool event_loop; // attributes indicates whether to continue the main loop

    /* Actor Behaviour Functions */
    void (*on_message)(struct __actor *self, MPI_Status *status);
    void (*execute_step)(struct __actor *self, int argc, char **argv);
    void (*new_actor)(char *type, int count);

    /* Actor Lifecycle Function */
    void (*pre_process)(struct __actor *self);
    void (*post_process)(struct __actor *self);
    void (*terminate)(struct __actor *self);
} ACTOR;

/* Framework Function */
/* Start actor */
void actor_start(struct __actor *actor);

#endif