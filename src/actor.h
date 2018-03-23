#ifndef __ACTOR_H
#define __ACTOR_H

#include <stdbool.h>
#include <mpi.h>

#include "configurations.h"

/* abstract actor definition */
typedef struct __actor
{
    char type[ACTOR_TYPE_NAME_LIMIT]; // actor type
    bool event_loop; // event loop control flag

    /* --------- Actor Behaviour Functions --------- */

    /* 
     * Behaviour after a message come
     * @param(self) : the pointer to current actor
     * @param(status) : pointer to MPI_Status obtained by MPI_Iprobe
     */
    void (*on_message)(struct __actor *self, MPI_Status *status);
    
    /* 
     * Single computation step within loops if no message come
     * @param(self) : the pointer to current actor
     * @param(argc) : count of parameters
     * @param(argv) : value of parameters
     */
    void (*execute_step)(struct __actor *self, int argc, char **argv);
    
    /* 
     * Create a new actor
     * @param(self) : the pointer to current actor
     * @param(type) : type of actor to be created
     * @param(count) : count of actor to be created
     */
    void (*new_actor)(struct __actor *self, char *type, int count);
    
    /* --------- End Actor Behaviour Functions --------- */

    /* --------- Actor Lifecycle Function --------- */

    /* 
     * Preprocessing, can be used for setup purpose
     * @param(self) : the pointer to current actor
     */
    void (*pre_process)(struct __actor *self);

    /* 
     * Postprocessing, can be used for teardown or one-off execution purpose
     * @param(self) : the pointer to current actor
     */
    void (*post_process)(struct __actor *self);

    /* 
     * Terminate current actor
     * @param(self) : the pointer to current actor
     */
    void (*terminate)(struct __actor *self);

    /* --------- End Actor Lifecycle Function --------- */
} ACTOR;


/* --------- Framework Function --------- */

/* 
 * Start actor after actor has been created 
 * @param(actor) : pointer to the created actor
 */
void actor_start(struct __actor *actor);

/* --------- Framework Function --------- */

#endif