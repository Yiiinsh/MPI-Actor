#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <mpi.h>

#include "actor.h"
#include "landcell_actor.h"
#include "pool.h"
#include "msg_tag.h"
#include "configurations.h"

void landcell_actor_on_message(ACTOR *actor, MPI_Status *status);
void landcell_actor_terminate(ACTOR *actor);

int infection_level[2];
int population_influx[3];

void create_landcell_actor(ACTOR *actor)
{
    strncpy(actor->type, "LANDCELL", ACTOR_TYPE_NAME_LIMIT);
    actor->event_loop = true;

    actor->on_message = &landcell_actor_on_message;
    actor->execute_step = NULL;
    actor->new_actor = NULL;

    actor->pre_process = NULL;
    actor->post_process = NULL;
    actor->terminate = &landcell_actor_terminate;

    memset(infection_level, 0, sizeof(int) * 2);
    memset(population_influx, 0, sizeof(int) * 3);
}

void landcell_actor_on_message(ACTOR *actor, MPI_Status *status)
{
    int count;
    int source = status->MPI_SOURCE;
    int tag = status->MPI_TAG;

    switch (tag)
    {
    case LANDCELL_QUERY_TAG:
    {
        int buf[5];
        buf[0] = infection_level[0], buf[1] = infection_level[1];
        buf[2] = population_influx[0], buf[3] = population_influx[1], buf[4] = population_influx[2];
        MPI_Recv(NULL, 0, MPI_INT, source, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Ssend(buf, 5, MPI_INT, source, tag, MPI_COMM_WORLD);
        break;
    }
    case LANDCELL_TERMINATE_TAG:
        actor->terminate(actor);
        break;
    default:
        break;
    }
}

void landcell_actor_terminate(ACTOR *actor)
{
    actor->event_loop = false;
}