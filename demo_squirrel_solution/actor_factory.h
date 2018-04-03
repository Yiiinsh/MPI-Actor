#ifndef __ACTOR_FACTORY_H
#define __ACTOR_FACTORY_H

#include "../actor_framework/actor.h"

/* --------- Customized Actors --------- */

void create_clock_actor(ACTOR *actor);
void create_landcell_actor(ACTOR *actor);
void create_squirrel_actor(ACTOR *actor);

/* --------- End Customized Actors --------- */

#endif