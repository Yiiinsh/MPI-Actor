/* Customized configurations */
#ifndef __CONFIGURATIONS_H
#define __CONFIGURATIONS_H

/* Default Framework Configurations, modified with cautions */
#define RANK_MAIN_ACTOR 0 // rank master a.k.a main actor rank
#define MPI_BUFFER_BYTES 10000000 // ~10mb
#define ACTOR_TYPE_NAME_LIMIT 64 // limit length of actor type
#define DEBUG 0
#define SIMULATION_ERROR 999

/* Problem specified configurations, customized by framework user */
/* Message Tag Config, Make sure no corrupt */
#define ACTOR_CREATE_TAG 1000
#define LANDCELL_QUERY_TAG 1100
#define LANDCELL_TERMINATE_TAG 1101
#define LANDCELL_ON_HOP_TAG 1102
#define SQUIRREL_BORN_TAG 1200
#define SQUIRREL_PREPROCESS_TAG 1201
#define SQUIRREL_TERMINATE_TAG 1202
#define SQUIRREL_INFECT_TAG 1203
#define SQUIRREL_HOP_TAG 1204

/* Problem Config */
#define LAND_CELL_COUNT 16
#define SQUIRREL_COUNT 34
#define SQUIRREL_LIMIT 200
#define SQUIRREL_INFECT_ON_INIT 4
#define HOP_HISTORY_SIZE 50
#define WILL_DEATH_AFTER_HOP 50
#define MONTH_LIMIT 24
#define MONTH_STEP 2
#define SQUIRREL_SLEEP_NANO 100000000

#endif