#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <mpi.h>

#include "../actor_framework/actor.h"
#include "../actor_framework/customized_actors.h"
#include "../actor_framework/pool.h"
#include "../actor_framework/configurations.h"
#include "squirrel-functions.h"

void squirrel_actor_on_message(ACTOR *actor, MPI_Status *status);
void squirrel_actor_execute_step(ACTOR *actor, int argc, char **argv);
void squirrel_actor_new_actor(ACTOR *actor, char *type, int count);
void squirrel_actor_pre_process(ACTOR *actor);
void squirrel_actor_terminate(ACTOR *actor);
void hoply_update(int new_infection_level, int new_population_influx);
float get_avg_population();
float get_avg_infection();

extern int rank;                                 // rank
long seed;                                       // random seed
float coordination[2] = {0.0, 0.0};              // squirrel coordination
int squirrel_landcell_to_rank[LAND_CELL_COUNT];  // landcell to rank map
int clock_rank;                                  // clock rank
int healthy = 1;                                 // healthy status
int current_steps = 0;                           // current step
int steps_after_infection = 0;                   // step after infection
int infection_level_history[HOP_HISTORY_SIZE];   // infection level history
int population_influx_history[HOP_HISTORY_SIZE]; // population level history

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

    seed = -1 - rank;
    initialiseRNG(&seed);
    memset(infection_level_history, 0, HOP_HISTORY_SIZE * sizeof(int));
    memset(population_influx_history, 0, HOP_HISTORY_SIZE * sizeof(int));
}

void squirrel_actor_on_message(ACTOR *actor, MPI_Status *status)
{
    int count;
    int source = status->MPI_SOURCE;
    int tag = status->MPI_TAG;

    switch (tag)
    {
    case SQUIRREL_INFECT_TAG: /* For initialize squirrel infection */
    {
        MPI_Recv(NULL, 0, MPI_INT, source, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        healthy = 0;
        break;
    }
    case SQUIRREL_TERMINATE_TAG:
        MPI_Recv(NULL, 0, MPI_INT, source, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        actor->terminate(actor);
        break;
    default:
        break;
    }
}

void squirrel_actor_execute_step(ACTOR *actor, int argc, char **argv)
{
    /* New coords */
    float new_x, new_y;
    squirrelStep(coordination[0], coordination[1], &new_x, &new_y, &seed);
    int landcell = getCellFromPosition(new_x, new_y);

    /* Update statistics */
    coordination[0] = new_x, coordination[1] = new_y;
    ++current_steps;
    if (!healthy)
    {
        ++steps_after_infection;
    }

    /* Send to landcells to declare a hop, receiving for infection_level and population_influx of the landcell */
    int infection_level, population_influx;
    int buf[2];
    if (DEBUG)
            fprintf(stdout, "[SQUIRREL %d] Step to %d\n", rank, squirrel_landcell_to_rank[landcell]);
    MPI_Ssend(&healthy, 1, MPI_INT, squirrel_landcell_to_rank[landcell], LANDCELL_ON_HOP_TAG, MPI_COMM_WORLD);
    MPI_Recv(buf, 2, MPI_INT, squirrel_landcell_to_rank[landcell], LANDCELL_ON_HOP_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    if (DEBUG)
            fprintf(stdout, "[SQUIRREL %d] Finish step to %d\n", rank, squirrel_landcell_to_rank[landcell]);
    infection_level = buf[0];
    population_influx = buf[1];

    /* Update history after every hop */
    hoply_update(infection_level, population_influx);

    /* Reproduce */
    if ((current_steps % 50) == 0 && willGiveBirth(get_avg_population(), &seed)) // may reproduce after every 50 steps
    {
        if (DEBUG)
            fprintf(stdout, "[SQUIRREL %d] Reproduce\n", rank);
        actor->new_actor(actor, "SQUIRREL", 1);
        /* Notify clock */
        MPI_Bsend(NULL, 0, MPI_INT, clock_rank, SQUIRREL_BORN_TAG, MPI_COMM_WORLD);
    }

    /* Infection */
    if (current_steps > 50 && healthy && willCatchDisease(get_avg_infection(), &seed))
    {
        if (DEBUG)
            fprintf(stdout, "[SQUIRREL %d] Infected\n", rank);
        healthy = 0;
        /* Notify clock */
        MPI_Bsend(NULL, 0, MPI_INT, clock_rank, SQUIRREL_INFECT_TAG, MPI_COMM_WORLD);
    }

    /* Death */
    if (!healthy && steps_after_infection >= WILL_DEATH_AFTER_HOP && willDie(&seed))
    {
        if (DEBUG)
            fprintf(stdout, "[SQUIRREL %d] Dead\n", rank);
        actor->terminate(actor);
        /* Notify clock */
        MPI_Bsend(NULL, 0, MPI_INT, clock_rank, SQUIRREL_TERMINATE_TAG, MPI_COMM_WORLD);
    }

    /* Squirrel sleep for nanosecs */
    struct timespec tim, tim2;
    tim.tv_sec = 0;
    tim.tv_nsec = SQUIRREL_SLEEP_NANO;
    nanosleep(&tim , &tim2);
}

void squirrel_actor_new_actor(ACTOR *actor, char *type, int count)
{
    int squirrel_rank;
    for (int i = 0; i < count; ++i)
    {
        /* Get available rank */
        squirrel_rank = startWorkerProcess();

        /* Pack coords, landcell_to_rank and clock_rank into one message  */
        float buf[2 + LAND_CELL_COUNT + 1];
        buf[0] = coordination[0], buf[1] = coordination[1];
        for (int i = 2; i < 2 + LAND_CELL_COUNT; ++i)
        {
            buf[i] = (float)squirrel_landcell_to_rank[i - 2];
        }
        buf[2 + LAND_CELL_COUNT] = (float)clock_rank;

        /* Create new squirrel and send args */
        MPI_Bsend("SQUIRREL", 9, MPI_CHAR, squirrel_rank, ACTOR_CREATE_TAG, MPI_COMM_WORLD);
        MPI_Bsend(buf, 2 + LAND_CELL_COUNT + 1, MPI_FLOAT, squirrel_rank, SQUIRREL_PREPROCESS_TAG, MPI_COMM_WORLD);
    }
}

void squirrel_actor_pre_process(ACTOR *actor)
{
    /* Get coords, landcell_to_rank and clock_rank */
    float buf[2 + LAND_CELL_COUNT + 1];
    MPI_Recv(buf, 2 + LAND_CELL_COUNT + 1, MPI_FLOAT, MPI_ANY_SOURCE, SQUIRREL_PREPROCESS_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    /* Decode */
    coordination[0] = buf[0], coordination[1] = buf[1];
    for (int i = 2; i < 2 + LAND_CELL_COUNT; ++i)
    {
        squirrel_landcell_to_rank[i - 2] = (int)buf[i];
    }
    clock_rank = (int)buf[2 + LAND_CELL_COUNT];
}

void squirrel_actor_terminate(ACTOR *actor)
{
    actor->event_loop = false;
}

/* Update infection level history and population influx history after every hop */
void hoply_update(int new_infection_level, int new_population_influx)
{
    infection_level_history[current_steps % HOP_HISTORY_SIZE] = new_infection_level;
    population_influx_history[current_steps % HOP_HISTORY_SIZE] = new_population_influx;
}

/* Average of population influx history */
float get_avg_population()
{
    float res = 0;
    for (int i = 0; i < HOP_HISTORY_SIZE; ++i)
    {
        res += population_influx_history[i];
    }
    res /= HOP_HISTORY_SIZE;
    return res;
}

/* Sum of infection history */
float get_avg_infection()
{
    float res = 0;
    for (int i = 0; i < HOP_HISTORY_SIZE; ++i)
    {
        res += infection_level_history[i];
    }
    res /= HOP_HISTORY_SIZE;
    return res;
}