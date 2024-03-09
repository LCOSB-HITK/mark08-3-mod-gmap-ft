/***
	author: Lucius Pertis
	Complete project details at https://github.com/LCOSB-HITK/
***/

#ifndef LCOSB_DATAOBJS_H
#define LCOSB_DATAOBJS_H

#include "lcosb_config_dev.h"
#include "Arduino.h"
#include "esp_err.h"

// bypassed for testing
//#include "lcosb_repointf.h"

// bypassing for testing
enum lcosb_repo_dtype_t {
	LCOSB_REPO_PL,
	LCOSB_REPO_GOBJ_TABLE,
	LCOSB_REPO_GOBJ_WALL,
	LCOSB_REPO_GOBJ_OBST,
	LCOSB_REPO_OBJ_BUNDLE,
	LCOSB_REPO_PL_BUNDLE,
	LCOSB_REPO_GOBJ_TABLE,
	LCOSB_REPO_GOBJ_WALL,
	LCOSB_REPO_GOBJ_OBST,
	LCOSB_REPO_UNIT_KINETIC,
}

/** lcosb_obj_base_t
 * 
*/
typedef struct {
	int		com_err[3];
	uint8_t	accuracy;
	uint16_t gtime;
	uint8_t tag;

	lcosb_repo_dtype_t dtype;

	lcosb_repo_id_t repo_id;

} lcosb_obj_base_t;

/** lcosb_objhint_t
 * 
*/
typedef struct {
	lcosb_obj_base_t reciever_base_hint;

	lcosb_repo_id_t	*data_objs;
	uint8_t 		 data_objs_len;	
} lcosb_objhint_t;

// MAJOR: pl migrated to lcosb_echo

/** lcosb_pl_bundle_t
 * 
*/
typedef struct {
	int com[3];

	uint8_t		pl_count;
	uint16_t	glen; // in mm
	uint16_t	pl_gtime_avg;

	lcosb_obj_base_t base;
} lcosb_pl_bundle_t;

/** lcosb_gobj_table_t
 * 
*/
typedef struct {
	int com[3];
	int	pl[2][3]; // left = 1, right = 0
	uint8_t 	pl_count[2];

	lcosb_obj_base_t base;
} lcosb_gobj_table_t;

/** lcosb_gobj_wall_t
 * 
 * - to be used in map pattern matching algo
*/
typedef struct {
	lcosb_pl_t *pl; // left = 1, right = 0
	uint8_t 	pl_len;

	bool has_corner;

	lcosb_obj_base_t base;
} lcosb_gobj_wall_t;

/** lcosb_gobj_obst_t
 * 
*/
typedef struct {
	int com[3];
	int (*llm)[2]; // x,y polygon

	uint8_t acc;

	lcosb_obj_base_t base;
} lcosb_gobj_obst_t;

/** lcosb_obj_bundle_t
 * 
*/
typedef struct lcosb_obj_bundle {
	lcosb_repo_id_t *obj_repo_ids;
	int (*obj_coms)[3];

	int obj_len;

	lcosb_repo_dtype_t obj_dtype;
	
	lcosb_obj_base_t base;
} lcosb_obj_bundle_t;

/** lcosb_repo_id_t
 * 
*/
typedef struct lcosb_repo_id {
	uint16_t	obj_logical_id;
	uint8_t		obj_shard_id;
} lcosb_repo_id_t;

/** lcosb_unit_kinetic_t
 * 
*/
typedef struct {
	int gpos[3];
	int gvel[3];
} lcosb_unit_kinetic_t;



#endif // LCOSB_DATAOBJS_H
