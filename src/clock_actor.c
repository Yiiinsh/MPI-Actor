#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <mpi.h>

#include "clock_actor.h"
#include "pool.h"
#include "msg_tag.h"
#include "configurations.h"

#define RANK_MAIN_ACTOR 0

#define MONTH_LIMIT 24
#define MONTH_STEP 1

void clock_actor_on_message(ACTOR *actor, MPI_Status *status);
void clock_actor_execute_step(ACTOR *actor, int argc, char **argv);
void clock_actor_pre_process(ACTOR *actor);
void clock_actor_terminate(ACTOR *actor);

int current_month = 0;
int landcell_to_rank[LAND_CELL_COUNT];

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
}

void clock_actor_on_message(ACTOR *actor, MPI_Status *status)
{
    int count;
    int source = status->MPI_SOURCE;
    int tag = status->MPI_TAG;

    switch (tag)
    {
    default:
        break;
    }
}

void clock_actor_execute_step(ACTOR *actor, int argc, char **argv)
{
    sleep(MONTH_STEP);

    int buf[5];
    fprintf(stdout, "[MONTH %3d]\n", current_month);
    fprintf(stdout, "%-25s%-25s%-25s%-25s%-25s\n", "infec1", "infec2", "population1", "population2", "population3");
    for (int i = 0; i < LAND_CELL_COUNT; ++i)
    {
        MPI_Ssend(NULL, 0, MPI_INT, landcell_to_rank[i], LANDCELL_QUERY_TAG, MPI_COMM_WORLD);
        MPI_Recv(buf, 5, MPI_INT, landcell_to_rank[i], LANDCELL_QUERY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        fprintf(stdout, "%-25d%-25d%-25d%-25d%-25d\n", buf[0], buf[1], buf[2], buf[3], buf[4]);
    }
    fprintf(stdout, "\n");

    ++current_month;
    if (current_month > MONTH_LIMIT)
    {
        /* Terminate landcells */
        MPI_Request requests[LAND_CELL_COUNT];
        for (int i = 0; i < LAND_CELL_COUNT; ++i)
        {
            MPI_Issend(NULL, 0, MPI_INT, landcell_to_rank[i], LANDCELL_TERMINATE_TAG, MPI_COMM_WORLD, &requests[i]);
        }
        MPI_Waitall(LAND_CELL_COUNT, requests, MPI_STATUS_IGNORE);

        actor->terminate(actor);
        shutdownPool();
    }
}

void clock_actor_pre_process(ACTOR *actor)
{
    /* Recv the rank of landcells */
    MPI_Recv(landcell_to_rank, LAND_CELL_COUNT, MPI_INT, RANK_MAIN_ACTOR, ACTOR_CREATE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

void clock_actor_terminate(ACTOR *actor)
{
    actor->event_loop = false;
}