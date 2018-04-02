#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <mpi.h>

#include "actor.h"
#include "customized_actors.h"
#include "pool.h"
#include "configurations.h"
#include "../demo_squirrel_solution/squirrel-functions.h"

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

extern int rank;
long seed;                             // radom number generator seeds
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

    seed = -1 - rank;
    initialiseRNG(&seed);
}

/* 
 * PreProcess for setup,  !!! customized it to create initial customized actors !!!
 * @param(actor): current actor
 */
void main_actor_pre_process(ACTOR *actor)
{
    /* Init necessary actors and ensure the start order */

    /* landcell actors */
    MPI_Request landcell_requests[LAND_CELL_COUNT];
    int landcell_rank;
    for (int i = 0; i < LAND_CELL_COUNT; ++i)
    {
        landcell_rank = startWorkerProcess();
        MPI_Issend("LANDCELL", 9, MPI_CHAR, landcell_rank, ACTOR_CREATE_TAG, MPI_COMM_WORLD, &landcell_requests[i]);
        landcell_to_rank[i] = landcell_rank;

        if (DEBUG)
            fprintf(stdout, "[MAIN] Create landcell on rank %d\n", landcell_rank);
    }
    MPI_Waitall(LAND_CELL_COUNT, landcell_requests, MPI_STATUS_IGNORE);

    /* clock actors and activate later */
    int clock_rank = startWorkerProcess();
    MPI_Ssend("CLOCK", 6, MPI_CHAR, clock_rank, ACTOR_CREATE_TAG, MPI_COMM_WORLD);

    /* squirrel actors */
    int squirrel_rank;
    
    /* coords, landcell_to_rank and clock_rank in one message */
    float buf[2 + LAND_CELL_COUNT + 1];
    buf[0] = 0.0, buf[1] = 0.0;
    for (int i = 2; i < 2 + LAND_CELL_COUNT; ++i)
    {
        buf[i] = (float)landcell_to_rank[i - 2];
    }
    buf[2 + LAND_CELL_COUNT] = (float)clock_rank;

    for (int i = 0; i < SQUIRREL_COUNT; ++i)
    {
        squirrel_rank = startWorkerProcess();
        MPI_Ssend("SQUIRREL", 9, MPI_CHAR, squirrel_rank, ACTOR_CREATE_TAG, MPI_COMM_WORLD);
        MPI_Ssend(buf, 2 + LAND_CELL_COUNT + 1, MPI_FLOAT, squirrel_rank, SQUIRREL_PREPROCESS_TAG, MPI_COMM_WORLD);

        // set init infected squirrels
        if (i < 4)
        {
            MPI_Ssend(NULL, 0, MPI_INT, squirrel_rank, SQUIRREL_INFECT_TAG, MPI_COMM_WORLD);
        }

        if (DEBUG)
            fprintf(stdout, "[MAIN] Create squirrel on rank %d\n", squirrel_rank);
    }

    /* Activate clock */
    MPI_Ssend(landcell_to_rank, LAND_CELL_COUNT, MPI_INT, clock_rank, ACTOR_CREATE_TAG, MPI_COMM_WORLD);
    if (DEBUG)
        fprintf(stdout, "[MAIN] Create clock on rank %d\n", clock_rank);
}

/* 
 * PostProcess for master poll
 * @param(actor): current actor
 */
void main_actor_post_process(ACTOR *actor)
{
    if (DEBUG)
        fprintf(stdout, "[MAIN %d] Postprocessing...\n", rank);
    int main_actor_status = masterPoll();
    while (main_actor_status)
    {
        main_actor_status = masterPoll();
    }
    if (DEBUG)
        fprintf(stdout, "[MAIN %d] Postprocessing done\n", rank);
}