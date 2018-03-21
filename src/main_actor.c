#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#include "actor.h"
#include "main_actor.h"
#include "msg_tag.h"
#include "pool.h"

#define BUF_LIMIT 1024

static void *get_attr(ACTOR *actor, char *key);
static void main_actor_on_message(MPI_Status *status);
static void main_actor_new_actor(char *type, int count);
static void terminate(ACTOR *actor);

void create_main_actor(ACTOR *actor)
{
    strncpy(actor->type, "MAIN", ACTOR_TYPE_NAME_LIMIT);
    actor->event_loop = true;

    actor->get_attr = &get_attr;

    actor->on_message = &main_actor_on_message;
    actor->execute_step = NULL;
    actor->new_actor = &main_actor_new_actor;

    actor->pre_process = NULL;
    actor->post_process = NULL;
    actor->terminate = &terminate;
}

void *get_attr(ACTOR *actor, char *key)
{
}

void main_actor_on_message(MPI_Status *status)
{
    int count;
    int source = status->MPI_SOURCE;
    int tag = status->MPI_TAG;

    switch (tag)
    {
    case MAIN_ACTOR_MSG_CREATE_TAG:
        /* Create message are composed of two args: type name and number */
        MPI_Get_count(status, MPI_CHAR, &count);
        char buf[BUF_LIMIT], type[BUF_LIMIT];
        MPI_Recv(buf, count, MPI_CHAR, source, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        char *msg[2];
        msg[0] = strtok(buf, ACTOR_MSG_DELIMITER);
        msg[1] = strtok(buf, ACTOR_MSG_DELIMITER);

        main_actor_new_actor(msg[0], atoi(msg[1]));
        break;
    case MAIN_ACTOR_MSG_TERMINATE_TAG:
        break;
    default:
        fprintf(stdout, "Ignore unknown tag:%d\n from rank %d\n", tag, source);
    }
}

void main_actor_new_actor(char *type, int count)
{
    int rank = startWorkerProcess();
    fprintf(stdout, "Start worker on rank %d\n", rank);
}

void terminate(ACTOR *actor)
{
}