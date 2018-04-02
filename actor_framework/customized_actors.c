#include <stdlib.h>
#include <string.h>

#include "customized_actors.h"

/* 
 * General Actors Creation Factory , extend it for specified usage
 * @param(type) : type of the actor to be created
 * @out(actor) : pointer to the created actor
 */
void create_actor(char *type, ACTOR *actor)
{
	if (NULL != actor)
	{
		if (strcmp(type, "CLOCK") == 0)
		{
			create_clock_actor(actor);
		}
		else if (strcmp(type, "LANDCELL") == 0)
		{
			create_landcell_actor(actor);
		}
		else if (strcmp(type, "SQUIRREL") == 0)
		{
			create_squirrel_actor(actor);
		}
	}
}