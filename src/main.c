#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#include "actor.h"
#include "pool.h"
#include "customized_actors.h"
#include "configurations.h"

char mpi_buf[MPI_BUFFER_BYTES];
int size, rank; // MPI info
ACTOR actor;    // Current Actor
int statusCode; // Process pool status code

/* Entry of the framework */
int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Buffer_attach(mpi_buf, MPI_BUFFER_BYTES);

    statusCode = processPoolInit();
    if (2 == statusCode) /* Master actor */
    {
        create_main_actor(&actor);
        actor_start(&actor);
    }
    else if (1 == statusCode) /* Worker Actor */
    {
        // Recv actor type
        char type[ACTOR_TYPE_NAME_LIMIT];
        memset(type, '\0', ACTOR_TYPE_NAME_LIMIT);
        MPI_Recv(type, ACTOR_TYPE_NAME_LIMIT, MPI_CHAR, RANK_MAIN_ACTOR, ACTOR_CREATE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        // create actor and start
        create_actor(type, &actor);
        actor_start(&actor);
    }

    int dummy_mpi_buf_size = MPI_BUFFER_BYTES;
    MPI_Buffer_detach(mpi_buf, &dummy_mpi_buf_size);
    processPoolFinalise();
    MPI_Finalize();
    return 0;
}