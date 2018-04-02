#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <mpi.h>

#include "../actor_framework/customized_actors.h"
#include "../actor_framework/pool.h"
#include "../actor_framework/configurations.h"

void clock_actor_on_message(ACTOR *actor, MPI_Status *status);
void clock_actor_execute_step(ACTOR *actor, int argc, char **argv);
void clock_actor_pre_process(ACTOR *actor);
void clock_actor_terminate(ACTOR *actor);

extern int rank;                                                                                                                     // rank
int current_month = 0;                                                                                                               // current month
int clock_landcell_to_rank[LAND_CELL_COUNT];                                                                                         // landcell_to_rank map
int health_count = SQUIRREL_COUNT - SQUIRREL_INFECT_ON_INIT, infected_count = SQUIRREL_INFECT_ON_INIT, total_count = SQUIRREL_COUNT; // statistics
time_t base, curr;                                                                                                                   // current time and month base time

void create_clock_actor(ACTOR *actor)
{
    strncpy(actor->type, "CLOCK", ACTOR_TYPE_NAME_LIMIT);
    actor->event_loop = true;

    actor->on_message = &clock_actor_on_message;
    actor->execute_step = &clock_actor_execute_step;
    actor->new_actor = NULL;

    actor->pre_process = &clock_actor_pre_process;
    actor->post_process = NULL;
    actor->terminate = &clock_actor_terminate;

    base = time(&base);
    curr = time(&curr);
}

void clock_actor_on_message(ACTOR *actor, MPI_Status *status)
{
    int count;
    int source = status->MPI_SOURCE;
    int tag = status->MPI_TAG;

    switch (tag)
    {
    case SQUIRREL_TERMINATE_TAG: /* Squirrel Terminate */
        MPI_Recv(NULL, 0, MPI_INT, source, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        --total_count;
        --infected_count;
        break;
    case SQUIRREL_INFECT_TAG: /* Squirrel Infected */
        MPI_Recv(NULL, 0, MPI_INT, source, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        --health_count;
        ++infected_count;
        break;
    case SQUIRREL_BORN_TAG: /* Squirrel Born */
        MPI_Recv(NULL, 0, MPI_INT, source, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        ++total_count;
        ++health_count;
        break;
    default:
        break;
    }
}

void clock_actor_execute_step(ACTOR *actor, int argc, char **argv)
{
    /* Month simulation */
    time(&curr);
    if (difftime(curr, base) >= MONTH_STEP) /* Month passed */
    {
        /* Update timestamp */
        time(&base);

        int buf[2];
        fprintf(stdout, "[MONTH %3d] Health: %d, Infected: %d, Total: %d\n", current_month, health_count, infected_count, total_count);
        fprintf(stdout, "%-25s%-25s%-25s\n", "landcell", "population_influx", "infection_level");
        /* Receive metrics and print */
        for (int i = 0; i < LAND_CELL_COUNT; ++i)
        {
            MPI_Ssend(NULL, 0, MPI_INT, clock_landcell_to_rank[i], LANDCELL_QUERY_TAG, MPI_COMM_WORLD);
            MPI_Recv(buf, 2, MPI_INT, clock_landcell_to_rank[i], LANDCELL_QUERY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            fprintf(stdout, "%-25d%-25d%-25d\n", i + 1, buf[1], buf[0]);
        }
        fprintf(stdout, "\n");

        ++current_month;
        if (current_month >= MONTH_LIMIT || total_count >= SQUIRREL_LIMIT)
        {
            actor->terminate(actor);
        }
    }
}

void clock_actor_pre_process(ACTOR *actor)
{
    /* Recv the rank of landcells */
    MPI_Recv(clock_landcell_to_rank, LAND_CELL_COUNT, MPI_INT, RANK_MAIN_ACTOR, ACTOR_CREATE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

void clock_actor_terminate(ACTOR *actor)
{
    shutdownPool();
    actor->event_loop = false;
    fprintf(stderr, "Simulation restrictions meet, aborting......\n");
    MPI_Abort(MPI_COMM_WORLD, SIMULATION_ERROR);
}