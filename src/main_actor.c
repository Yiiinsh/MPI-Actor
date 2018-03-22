#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <mpi.h>

#include "actor.h"
#include "main_actor.h"
#include "pool.h"
#include "configurations.h"

void main_actor_pre_process(ACTOR *actor);
void main_actor_post_process(ACTOR *actor);

int landcell_to_rank[LAND_CELL_COUNT]; // mapping land cell id to rank

void create_main_actor(ACTOR *actor)
{
    strncpy(actor->type, "MAIN", ACTOR_TYPE_NAME_LIMIT);
    actor->event_loop = false;

    actor->on_message = NULL;
    actor->execute_step = NULL;
    actor->new_actor = NULL;

    actor->pre_process = &main_actor_pre_process;
    actor->post_process = &main_actor_post_process;
    actor->terminate = NULL;

    memset(landcell_to_rank, -1, sizeof(int) * LAND_CELL_COUNT);
}

/* Main actor does not support event loop, pre_process for simulation set up */
void main_actor_pre_process(ACTOR *actor)
{
    /* Init necessary actors in a blocking way to ensure the start order */
    int landcell_rank;
    for (int i = 0; i < LAND_CELL_COUNT; ++i)
    {
        landcell_rank = startWorkerProcess();
        MPI_Ssend("LANDCELL", 9, MPI_CHAR, landcell_rank, ACTOR_CREATE_TAG, MPI_COMM_WORLD);
        landcell_to_rank[i] = landcell_rank;
    }

    int clock_rank = startWorkerProcess();
    MPI_Ssend("CLOCK", 6, MPI_CHAR, clock_rank, ACTOR_CREATE_TAG, MPI_COMM_WORLD);
    MPI_Ssend(landcell_to_rank, LAND_CELL_COUNT, MPI_INT, clock_rank, ACTOR_CREATE_TAG, MPI_COMM_WORLD);
    // get worker actor rank and send them their type
}

/* Main actor does not support event loop, post_process for master poll */
void main_actor_post_process(ACTOR *actor)
{
    int main_actor_status = masterPoll();
    while (main_actor_status)
    {
        main_actor_status = masterPoll();
    }
}