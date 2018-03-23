#ifndef __CUSTOMIZED_ACTORS_H
#define __CUSTOMIZED_ACTORS_H

#include "actor.h"

/* 
 * General Actors Creation Factory 
 * @param(type) : type of the actor to be created
 * @out(actor) : pointer to the created actor
 */
void create_actor(char *type, ACTOR *actor);

/* --------- Customized Actors --------- */

void create_main_actor(ACTOR *actor);
void create_clock_actor(ACTOR *actor);
void create_landcell_actor(ACTOR *actor);
void create_squirrel_actor(ACTOR *actor);

/* --------- End Customized Actors --------- */

#endif