#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <mpi.h>

#include "actor.h"
#include "customized_actors.h"
#include "pool.h"
#include "configurations.h"
#include "squirrel-functions.h"

/* 
 * PreProcess for setup,  !!! customized it to create initial customized actors !!!
 * @param(actor): current actor
 */
void main_actor_pre_process(ACTOR *actor);

/* 
 * PostProcess for master poll
 * @param(actor): current actor
 */
void main_actor_post_process(ACTOR *actor);

long seed; // radom number generator seeds
int landcell_to_rank[LAND_CELL_COUNT]; // mapping landcell id to rank

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

    int main_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &main_rank);
    seed = -1 - main_rank;
    initialiseRNG(&seed);
}

/* 
 * PreProcess for setup,  !!! customized it to create initial customized actors !!!
 * @param(actor): current actor
 */
void main_actor_pre_process(ACTOR *actor)
{
    /* Init necessary actors in a blocking way to ensure the start order */

    /* landcell actors */
    MPI_Request landcell_requests[LAND_CELL_COUNT];
    int landcell_rank;
    for (int i = 0; i < LAND_CELL_COUNT; ++i)
    {
        landcell_rank = startWorkerProcess();
        MPI_Issend("LANDCELL", 9, MPI_CHAR, landcell_rank, ACTOR_CREATE_TAG, MPI_COMM_WORLD, &landcell_requests[i]);
        landcell_to_rank[i] = landcell_rank;
    }
    MPI_Waitall(LAND_CELL_COUNT, landcell_requests, MPI_STATUS_IGNORE);

    /* squirrel actors */
    MPI_Request squirrel_requests[SQUIRREL_COUNT * 3];
    int squirrel_rank;
    for (int i = 0; i < SQUIRREL_COUNT; ++i)
    {
        squirrel_rank = startWorkerProcess();
        MPI_Issend("SQUIRREL", 9, MPI_CHAR, squirrel_rank, ACTOR_CREATE_TAG, MPI_COMM_WORLD, &squirrel_requests[3 * i]);
        
        // send init coords
        float coordination[2];
        squirrelStep(0, 0, &coordination[0], &coordination[1], &seed);
        MPI_Issend(coordination, 2, MPI_FLOAT, squirrel_rank, SQUIRREL_PREPROCESS_TAG, MPI_COMM_WORLD, &squirrel_requests[3 * i + 1]);
        // send landcell_to_rank
        MPI_Issend(landcell_to_rank, LAND_CELL_COUNT, MPI_INT, squirrel_rank, SQUIRREL_PREPROCESS_TAG, MPI_COMM_WORLD, &squirrel_requests[3 * i + 2]);
    }
    MPI_Waitall(SQUIRREL_COUNT * 3, squirrel_requests, MPI_STATUS_IGNORE);

    /* clock actors */
    int clock_rank = startWorkerProcess();
    MPI_Ssend("CLOCK", 6, MPI_CHAR, clock_rank, ACTOR_CREATE_TAG, MPI_COMM_WORLD);
    MPI_Ssend(landcell_to_rank, LAND_CELL_COUNT, MPI_INT, clock_rank, ACTOR_CREATE_TAG, MPI_COMM_WORLD);
}

/* 
 * PostProcess for master poll
 * @param(actor): current actor
 */
void main_actor_post_process(ACTOR *actor)
{
    int main_actor_status = masterPoll();
    while (main_actor_status)
    {
        main_actor_status = masterPoll();
    }
}