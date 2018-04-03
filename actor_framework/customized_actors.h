#ifndef __CUSTOMIZED_ACTORS_H
#define __CUSTOMIZED_ACTORS_H

#include "actor.h"

/* 
 * General Actors Creation Factory , customized it in your solution
 * @param(type) : type of the actor to be created
 * @out(actor) : pointer to the created actor
 */
void create_actor(char *type, ACTOR *actor);

/* 
 * Main actor creations
 * @param(actor) : main actor
 */
void create_main_actor(ACTOR *actor);


#endif