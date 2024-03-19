/***
	author: Lucius Pertis
	Complete project details at https://github.com/LCOSB-HITK/
***/

#ifndef SIMPLE_GMAP_H
#define SIMPLE_GMAP_H

#include "lcosb_config_dev.h"
#include "Arduino.h"
#include "esp_err.h"

#include "lcosb_echo.h"

enum OBJ_TYPE {
	TYPE_OBJ = 0,
	TYPE_OBJ_TABLE = 1,
	TYPE_PLB,
	TYPE_MAPFRAG,
	TYPE_GMAP
};

typedef struct {
	int bounds[2][2];
	int frags[16];
	int frag_count;

	int earliest_upd_time;
	int latest_upd_time;

	int gmap_id; 
} simple_gmap_t;

typedef struct {
	int tables[8][4]; // {obj_id, com[0:3]}
	int table_count;

	int bounds[2][2];

	int plb_store_tag;

	int earliest_upd_time;
	int latest_upd_time;

	int frag_id;
} simple_gmap_mapfrag_t;

typedef struct {
	lcosb_echo_pl_t* pls[8];
	int pl_count;

	int reff_obj_id;

	int plb_id;
} simple_gmap_plb_t;

typedef struct {
	int com[3];

	int llm[4][2];

	int gtime;
	int acc;

	int obj_id;
} simple_gmap_obj_table_t;

// singular gmap and mapfrag for testing
static simple_gmap_t sgmap_GMAP;
static simple_gmap_mapfrag_t sgmap_MAPFRAG[5];

// dynamic list for obj_table
static simple_gmap_obj_table_t sgmap_OBJ_TABLE[16];
static simple_gmap_pl_b_t sgmap_ua_PL_B[16];

// simple create funtions
int simple_gmap_createGMap();
int simple_gmap_createMapFrag(int gmap_id);
int simple_gmap_createTable(int frag_id);


// simple equating and merging functions
int simple_gmap_eqGMap(simple_gmap_t *gmap1, simple_gmap_t *gmap2);
int simple_gmap_eqMapFrag(simple_gmap_mapfrag_t *mapfrag1, simple_gmap_mapfrag_t *mapfrag2);
int simple_gmap_mergeMapFrag(simple_gmap_mapfrag_t *mapfrag1, simple_gmap_mapfrag_t *mapfrag2);

int simple_gmap_eqTable(int *table1, int *table2);
int simple_gmap_mergeTable(int *table1, int *table2);


// simple pl_b 2 mapfrag mapping functions
int simple_gmap_mapPLB2MapFrag(int pl_b_id, int frag_id);

// simple recompose mapfrag functions
int simple_gmap_recomposeMapFrag(int frag_id);
int simple_gmap_recomposeObjTable(int frag_id, int obj_id);

void simple_gmap_recalcObjTable(simple_gmap_obj_table_t* obj_table, int COM2LLM);

// net CRUD functions

// create/update/publish
int simple_gmap_publishObj(int id, const char* serialized_obj);

// read
int simple_gmap_readObj(int id, char* buffer);

// delete
int simple_gmap_deleteObj(int id);


// net gmap objects serialization and deserialization
int simple_gmap_serializeGMap(simple_gmap_t *gmap, char *buffer);
int simple_gmap_deserializeGMap(simple_gmap_t *gmap, char *buffer);

int simple_gmap_serializeMapFrag(simple_gmap_mapfrag_t *mapfrag, char *buffer);
int simple_gmap_deserializeMapFrag(simple_gmap_mapfrag_t *mapfrag, char *buffer);

int simple_gmap_serializeObjTable(int *obj_table, char *buffer);
int simple_gmap_deserializeObjTable(int *obj_table, char *buffer);

int simple_gmap_serializePLB(int *pl_b, char *buffer);
int simple_gmap_deserializePLB(int *pl_b, char *buffer);




#endif // SIMPLE_GMAP_H
