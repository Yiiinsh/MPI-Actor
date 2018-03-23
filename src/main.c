#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>

#include "actor.h"
#include "pool.h"
#include "main_actor.h"
#include "clock_actor.h"
#include "landcell_actor.h"
#include "squirrel_actor.h"
#include "configurations.h"
#include "squirrel-functions.h"

#define RANK_MASTER 0
#define TYPE_NAME_BUF 64

int size, rank; // MPI info
ACTOR actor;    // Current Actor
int statusCode; // Process pool status code

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    statusCode = processPoolInit();
    if (2 == statusCode) /* Master actor */
    {
        create_main_actor(&actor);
        actor_start(&actor);
    }
    else if (1 == statusCode) /* Worker Actor */
    {
        char type[TYPE_NAME_BUF];
        memset(type, '\0', TYPE_NAME_BUF);
        MPI_Recv(type, TYPE_NAME_BUF, MPI_CHAR, RANK_MASTER, ACTOR_CREATE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (strcmp(type, "CLOCK") == 0)
        {
            create_clock_actor(&actor);
        }
        else if (strcmp(type, "LANDCELL") == 0)
        {
            create_landcell_actor(&actor);
        }
        else if (strcmp(type, "SQUIRREL") == 0)
        {
            create_squirrel_actor(&actor);
        }
        actor_start(&actor);
    }

    processPoolFinalise();
    MPI_Finalize();
    return 0;
}