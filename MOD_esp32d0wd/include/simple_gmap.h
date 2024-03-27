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


#define GMAP_MF_PER_GMAP_SIZE 16
#define GMAP_TABLES_PER_MF_SIZE 8
#define GMAP_PLS_PER_PLB_SIZE 8

#define GMAP_LOCAL_MF_SIZE 5
#define GMAP_LOCAL_OBJ_TABLE_SIZE 16
#define GMAP_LOCAL_UAPLB_SIZE 16
#define GMAP_LOCAL_OBJPLB_SIZE 16

enum OBJ_TYPE {
	TYPE_OBJ = 0,
	TYPE_OBJ_TABLE = 1,
	TYPE_PLB,
	TYPE_MAPFRAG,
	TYPE_GMAP
};

typedef struct {
	int bounds[2][2];
	int frags[GMAP_MF_PER_GMAP_SIZE];
	int frag_count;

	int earliest_upd_time;
	int latest_upd_time;

	int gmap_id; 
} simple_gmap_t;

typedef struct {
	int tables[GMAP_TABLES_PER_MF_SIZE][4]; // {obj_id, com[0:3]}
	int table_count;

	int bounds[2][2];

	int plb_store_tag;

	// gtime of tables
	int earliest_upd_time;
	int latest_upd_time;

	int frag_id;
} simple_gmap_mapfrag_t;

typedef struct {
	lcosb_echo_pl_t* pls[GMAP_PLS_PER_PLB_SIZE];
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
static simple_gmap_t 			sgmap_GMAP;
static simple_gmap_mapfrag_t	sgmap_MAPFRAG[GMAP_LOCAL_MF_SIZE];

// dynamic list for obj_table
static simple_gmap_obj_table_t	sgmap_OBJ_TABLE[GMAP_LOCAL_OBJ_TABLE_SIZE];

/**
 * plb_id == 0 => un-initialised plb
 * plb_id >= 1 => mapfrag assigned plb
 * plb_id == -1 => un-assigned plb (to be assigned / fresh)
 * plb_id == -2 => no mapfrag found; tried to assign * 
*/
static simple_gmap_plb_t		sgmap_ua_PL_B[GMAP_LOCAL_UAPLB_SIZE];

/**
 * plb_id == 0 => un-initialised plb
 * plb_id >= 1 => object assigned plb (reff_obj_id -> obj_id)
 * plb_id == -1 => no object found but belongs to mapfrag (reff_obj_id -> frag_id)
*/
static simple_gmap_plb_t		sgmap_obj_PL_B[GMAP_LOCAL_OBJPLB_SIZE];

// simple create funtions
int simple_gmap_createGMap(simple_gmap_t &gmap);
int simple_gmap_createMapFrag(int gmap_id, simple_gmap_mapfrag_t &mapfrag);
int simple_gmap_createTable(int frag_id, simple_gmap_obj_table_t &table);


// simple equating and merging functions
int simple_gmap_eqGMap(simple_gmap_t *gmap1, simple_gmap_t *gmap2);
int simple_gmap_eqMapFrag(simple_gmap_mapfrag_t *mapfrag1, simple_gmap_mapfrag_t *mapfrag2);
int simple_gmap_mergeMapFrag(simple_gmap_mapfrag_t *mapfrag1, simple_gmap_mapfrag_t *mapfrag2);

int simple_gmap_eqTable(int *table1, int *table2);
int simple_gmap_mergeTable(int *table1, int *table2);

// update functions (L0 -> L1)
int simple_gmap_updateGMap(int mapfrag_id);
int simple_gmap_updateMapFrag(int obj_table_id);

// refresh functions (L1 -> L0)
int simple_gmap_refreshMapFrags(int gmap_id);
int simple_gmap_refreshObjTables(int mapfrag_id);


// simple plb 2 mapfrag mapping functions
int simple_gmap_mapUaPLB2MapFrag(int plb_id, int frag_id);
int assignUaPLB2MapFrag(int local_plb_idx, int* member_marker, int assigned_frag_id);

// simple recompose mapfrag functions
int simple_gmap_recomposeMapFrag(int frag_id);
int simple_gmap_recomposeObjTable(int obj_id, int frag_id);

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

int simple_gmap_serializePLB(int *plb, char *buffer);
int simple_gmap_deserializePLB(int *plb, char *buffer);

int simple_gmap_compare_withinBounds(int **bounds, int *com)


#endif // SIMPLE_GMAP_H
