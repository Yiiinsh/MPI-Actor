#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <mpi.h>

#include "actor.h"
#include "customized_actors.h"
#include "pool.h"
#include "configurations.h"

void landcell_actor_on_message(ACTOR *actor, MPI_Status *status);
void landcell_actor_terminate(ACTOR *actor);
void monthly_update();
int get_infection_level();
int get_population_influx();

extern int rank;
int month; // current month
int current_infection_level; // infection_level this month
int current_population_influx; // population_influx this month
int infection_level[2]; // infection level history
int population_influx[3]; // population influx history

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

    int month = 0;
    int current_infection_level = 0;
    int current_population_influx = 0;
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
    case LANDCELL_ON_HOP_TAG: /* Squirrel hop in */
    {
        int health; // squirrel status
        int buf[2]; // send buf
        buf[0] = get_infection_level();
        buf[1] = get_population_influx();

        /* Receive on_hop message and send infection_level and population_influx back */
        MPI_Recv(&health, 1, MPI_INT, source, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Bsend(buf, 2, MPI_INT, source, tag, MPI_COMM_WORLD);

        /* Update statistics */
        ++current_population_influx;
        if (!health)
        {
            ++current_infection_level;
        }

        break;
    }
    case LANDCELL_QUERY_TAG: /* Clock query every month */
    {
        /* Update history records */
        monthly_update();

        int buf[2]; // send buf
        buf[0] = get_infection_level();
        buf[1] = get_population_influx();

        /* Receive query message and send metric<infection_level, population> back  */
        MPI_Recv(NULL, 0, MPI_INT, source, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Bsend(buf, 2, MPI_INT, source, LANDCELL_QUERY_TAG, MPI_COMM_WORLD);

        /* Update statistics */
        ++month;
        current_infection_level = 0;
        current_population_influx = 0;

        break;
    }
    case LANDCELL_TERMINATE_TAG:
        MPI_Recv(NULL, 0, MPI_INT, source, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
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

/* Update history records, first 3 month to init records, leftshift afterwards to keep the latest n month figures */
void monthly_update()
{
    if (0 == month)
    {
        infection_level[0] = current_infection_level;
        population_influx[0] = current_population_influx;
    }
    else if (1 == month)
    {
        infection_level[1] = current_infection_level;
        population_influx[1] = current_population_influx;
    }
    else if (2 == month)
    {
        infection_level[0] = infection_level[1], infection_level[1] = current_infection_level;
        population_influx[2] = current_population_influx;
    }
    else
    {
        infection_level[0] = infection_level[1], infection_level[1] = current_infection_level;
        population_influx[0] = population_influx[1], population_influx[1] = population_influx[2], population_influx[2] = current_population_influx;
    }
}

/* Get infection level for latest 2 months */
int get_infection_level()
{
    return infection_level[0] + infection_level[1];
}

/* Get population influx for latest 3 months */
int get_population_influx()
{
    return population_influx[0] + population_influx[1] + population_influx[2];
}