#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#include "actor.h"
#include "pool.h"
#include "main_actor.h"

#define RANK_MASTER 0

int size, rank; // MPI info
ACTOR actor;    // Current Actor
int statusCode; // Process pool status code

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    statusCode = processPoolInit();
    MPI_Barrier(MPI_COMM_WORLD);

    if (1 == statusCode)
    {
        create_main_actor(&actor);
        actor.new_actor(NULL, 1);
        actor_start(&actor);
    }
    else if (2 == statusCode)
    {
        fprintf(stdout, "Worker process on rank %d\n", rank);
    }

    processPoolFinalise();
    MPI_Finalize();
    return 0;
}