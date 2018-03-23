#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <mpi.h>

#include "actor.h"
#include "main_actor.h"
#include "pool.h"
#include "configurations.h"
#include "squirrel-functions.h"

void main_actor_pre_process(ACTOR *actor);
void main_actor_post_process(ACTOR *actor);

int main_rank;
long seed;
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

    MPI_Comm_rank(MPI_COMM_WORLD, &main_rank);
    seed = -1 - main_rank;
    initialiseRNG(&seed);
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
    int squirrel_rank;
    for (int i = 0; i < SQUIRREL_COUNT; ++i)
    {
        squirrel_rank = startWorkerProcess();
        MPI_Ssend("SQUIRREL", 9, MPI_CHAR, squirrel_rank, ACTOR_CREATE_TAG, MPI_COMM_WORLD);
        float coordination[2];
        squirrelStep(0, 0, &coordination[0], &coordination[1], &seed);
        // send the coords
        MPI_Ssend(coordination, 2, MPI_FLOAT, squirrel_rank, SQUIRREL_PREPROCESS_TAG, MPI_COMM_WORLD);
        // send landcell_to_rank
        MPI_Ssend(landcell_to_rank, LAND_CELL_COUNT, MPI_INT, squirrel_rank, SQUIRREL_PREPROCESS_TAG, MPI_COMM_WORLD);
    }
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