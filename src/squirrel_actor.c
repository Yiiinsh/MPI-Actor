#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <mpi.h>

#include "actor.h"
#include "landcell_actor.h"
#include "pool.h"
#include "msg_tag.h"
#include "configurations.h"

void create_squirrel_actor(ACTOR *actor)
{
    strncpy(actor->type, "SQUIRREL", ACTOR_TYPE_NAME_LIMIT);
    actor->event_loop = true;

    actor->on_message = NULL;
    actor->execute_step = NULL;
    actor->new_actor = NULL;

    actor->pre_process = NULL;
    actor->post_process = NULL;
    actor->terminate = NULL;
}