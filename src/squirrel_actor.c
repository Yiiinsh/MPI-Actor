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

void squirrel_actor_on_message(ACTOR *actor, MPI_Status *status);
void squirrel_actor_execute_step(ACTOR *actor, int argc, char **argv);
void squirrel_actor_new_actor(ACTOR *actor, char *type, int count);
void squirrel_actor_pre_process(ACTOR *actor);
void squirrel_actor_terminate(ACTOR *actor);

long seed;
float coordination[2] = {0.0, 0.0};
int landcell_to_rank[LAND_CELL_COUNT];
bool healthy = true;
int current_steps = 0;
int steps_after_infection = 0;
int population_influx_history[HOP_HISTORY_SIZE];
int infection_level_history[HOP_HISTORY_SIZE];

void create_squirrel_actor(ACTOR *actor)
{
    strncpy(actor->type, "SQUIRREL", ACTOR_TYPE_NAME_LIMIT);
    actor->event_loop = true;

    actor->on_message = &squirrel_actor_on_message;
    actor->execute_step = &squirrel_actor_execute_step;
    actor->new_actor = &squirrel_actor_new_actor;

    actor->pre_process = &squirrel_actor_pre_process;
    actor->post_process = NULL;
    actor->terminate = &squirrel_actor_terminate;

    memset(population_influx_history, 0, HOP_HISTORY_SIZE * sizeof(int));
    memset(infection_level_history, 0, HOP_HISTORY_SIZE * sizeof(int));

    int squirrel_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &squirrel_rank);
    seed = -1 - squirrel_rank;
    initialiseRNG(&seed);
}

void squirrel_actor_on_message(ACTOR *actor, MPI_Status *status)
{
    int count;
    int source = status->MPI_SOURCE;
    int tag = status->MPI_TAG;

    switch (tag)
    {
    case SQUIRREL_TERMINATE_TAG:
        actor->terminate(actor);
        break;
    default:
        break;
    }
}

void squirrel_actor_execute_step(ACTOR *actor, int argc, char **argv)
{
    float new_x, new_y;
    squirrelStep(coordination[0], coordination[1], &new_x, &new_y, &seed);
    int landcell = getCellFromPosition(new_x, new_y);

    // send to landcells to declare a hop
    MPI_Bsend(&healthy, 1, MPI_INT, landcell_to_rank[landcell], LANDCELL_ON_HOP_TAG, MPI_COMM_WORLD);

    int infection_level, population_influx;
    MPI_Recv(&infection_level, 1, MPI_INT, landcell_to_rank[landcell], LANDCELL_ON_HOP_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&population_influx, 1, MPI_INT, landcell_to_rank[landcell], LANDCELL_ON_HOP_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // infection level

    if (current_steps >= HOP_HISTORY_SIZE)
    {
        float avg_population_influx = 0;
        for (int i = 0; i < HOP_HISTORY_SIZE; ++i)
        {
            avg_population_influx += population_influx_history[i];
        }
        avg_population_influx /= HOP_HISTORY_SIZE;
        if (willGiveBirth(avg_population_influx, &seed))
        {
            actor->new_actor(actor, "SQUIRREL", 1);
        }

        float avg_infection_level = 0;
        for (int i = 0; i < HOP_HISTORY_SIZE; ++i)
        {
            avg_infection_level += infection_level_history[i];
        }
        avg_infection_level /= HOP_HISTORY_SIZE;
        if (willCatchDisease(avg_infection_level, &seed))
        {
            healthy = false;
        }
    }

    // death judgement
    if (steps_after_infection >= WILL_DEATH_AFTER_HOP)
    {
        if (willDie(&seed))
        {
            actor->terminate(actor);
        }
    }

    // update status
    if (current_steps > HOP_HISTORY_SIZE)
    {
        for (int i = 0; i < HOP_HISTORY_SIZE - 1; ++i)
        {
            infection_level_history[i] = infection_level_history[i + 1];
            population_influx_history[i] = population_influx_history[i + 1];
        }
        infection_level_history[HOP_HISTORY_SIZE - 1] = infection_level;
        population_influx_history[HOP_HISTORY_SIZE - 1] = population_influx;
    }
    else
    {
        infection_level_history[current_steps] = infection_level;
        population_influx_history[current_steps] = population_influx;
    }
    coordination[0] = new_x, coordination[1] = new_y;
    ++current_steps;
    if (!healthy)
        ++steps_after_infection;
}

void squirrel_actor_new_actor(ACTOR *actor, char *type, int count)
{
    if (strcmp(actor->type, "SQUIRREL") == 0 && strcmp(actor->type, "SQUIRREL") == 0)
    {
        MPI_Request *squirrel_requests = (MPI_Request *) malloc(sizeof(MPI_Request) * 3 * count);
        int squirrel_rank;
        for (int i = 0; i < count; ++i)
        {
            squirrel_rank = startWorkerProcess();
            MPI_Issend("SQUIRREL", 9, MPI_CHAR, squirrel_rank, ACTOR_CREATE_TAG, MPI_COMM_WORLD, &squirrel_requests[3 * i]);
            // send the coords
            MPI_Issend(coordination, 2, MPI_FLOAT, squirrel_rank, SQUIRREL_PREPROCESS_TAG, MPI_COMM_WORLD, &squirrel_requests[3 * i + 1]);
            // send landcell_to_rank
            MPI_Issend(landcell_to_rank, LAND_CELL_COUNT, MPI_INT, squirrel_rank, SQUIRREL_PREPROCESS_TAG, MPI_COMM_WORLD, &squirrel_requests[3 * i + 2]);
        }
        MPI_Waitall(3 * count, squirrel_requests, MPI_STATUS_IGNORE);
        free(squirrel_requests);
    }
}

void squirrel_actor_pre_process(ACTOR *actor)
{
    /* Recv coords */
    MPI_Recv(coordination, 2, MPI_FLOAT, MPI_ANY_SOURCE, SQUIRREL_PREPROCESS_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    /* Recv landcell_to_rank */
    MPI_Recv(landcell_to_rank, LAND_CELL_COUNT, MPI_INT, MPI_ANY_SOURCE, SQUIRREL_PREPROCESS_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

void squirrel_actor_terminate(ACTOR *actor)
{
    actor->event_loop = false;
}