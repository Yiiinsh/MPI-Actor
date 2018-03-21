#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#include "actor.h"

/* General actor behaviour process, call to start an actor */
void actor_start(struct __actor *actor)
{
    if (NULL == actor)
    {
        fprintf(stderr, "Starting a null actor...\n");
        return;
    }

    if (NULL != actor->pre_process)
    {
        actor->pre_process(actor);
    }

    MPI_Status status;
    int flag = 0;
    while (actor->event_loop)
    {
        while (!flag & NULL != actor->execute_step)
        {
            MPI_IProbe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
            actor->execute_step(0, NULL);
        }
        flag = 0;
        
        actor->on_message(&status);
    }

    if (NULL != actor->post_process)
    {
        actor->post_process(actor);
    }
}