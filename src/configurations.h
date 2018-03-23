/* Customized configurations */
#ifndef __CONFIGURATIONS_H
#define __CONFIGURATIONS_H

/* Default Framework Configurations, modified with cautions */
#define RANK_MAIN_ACTOR 0 // rank master a.k.a main actor rank
#define MPI_BUFFER_BYTES 100000000 // ~100mb
#define ACTOR_TYPE_NAME_LIMIT 64 // limit length of actor type

/* Problem specified configurations, customized by framework user */
/* Message Tag Config */
#define ACTOR_CREATE_TAG 1222
#define LANDCELL_QUERY_TAG 1233
#define LANDCELL_TERMINATE_TAG 1234
#define LANDCELL_ON_HOP_TAG 1235
#define SQUIRREL_PREPROCESS_TAG 1333
#define SQUIRREL_TERMINATE_TAG 1334

/* Problem Config */
#define LAND_CELL_COUNT 16
#define SQUIRREL_COUNT 40
#define HOP_HISTORY_SIZE 50
#define WILL_DEATH_AFTER_HOP 50
#define MONTH_LIMIT 24
#define MONTH_STEP 1

#endif