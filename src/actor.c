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
        while (!flag && actor->event_loop)
        {
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
            if (NULL != actor->execute_step)
            {
                actor->execute_step(actor, 0, NULL);
            }
        }
        flag = 0;

        if (NULL != actor->on_message)
        {
            actor->on_message(actor, &status);
        }
    }

    if (NULL != actor->post_process)
    {
        actor->post_process(actor);
    }
}