#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <mpi.h>


#include "actor.h"
#include "customized_actors.h"
#include "pool.h"
#include "configurations.h"

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