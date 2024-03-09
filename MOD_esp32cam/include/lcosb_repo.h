/***
	author: Lucius Pertis
	Complete project details at https://github.com/LCOSB-HITK/
***/

#ifndef LCOSB_REPOINTF_H
#define LCOSB_REPOINTF_H

#include "lcosb_config_dev.h"
#include "Arduino.h"
#include "esp_err.h"
#inlcude "lcosb_sys"

#include "lcosb_dataobjs.h"  

typedef enum {
	REPO_DTYPE_OBS_BUNDLE,
	REPO_DTYPE_PL,
	REPO_DTYPE_PL_BUNDLE,

	REPO_DTYPE_IMG_RAW,
	REPO_DTYPE_IMG_INF_ENV,
	REPO_DTYPE_IMG_INF_CH,
	REPO_DTYPE_IMG_INF_BUNDLE,

	REPO_DTYPE_OBJ_TABLE,
	REPO_DTYPE_OBJ_CHAIR,
	REPO_DTYPE_OBJ_WALL,
	REPO_DTYPE_OBJ_OBST,
	REPO_DTYPE_OBJ_BUNDLE,
	REPO_DTYPE_COM_BUNDLE,
	

	REPO_DTYPE_CT_GRP,
	REPO_DTYPE_CT_REG,

	REPO_DTYPE_GMAP,
	REPO_DTYPE_MAPFRAG,
	REPO_DTYPE_,
	REPO_DTYPE_,
	REPO_DTYPE_,
	REPO_DTYPE_,
} lcosb_repo_dtype_t;

typedef enum {
	REPO_UPDTYPE_NEW,
	REPO_UPDTYPE_REWRITE,
	REPO_UPDTYPE_APPEND,
	REPO_UPDTYPE_IN_CACHE,
	REPO_UPDTYPE_WRITE_THROUGH,
	REPO_UPDTYPE_SQ
} lcosb_repo_updtype_t;

typedef struct {
	lcosb_repo_id_t	repo_id;
	void* 			in_ram_data_p;


} lcosb_repo_dataobj_registry_t;

typedef struct {
	lcosb_repo_id_t				repo_ids;
	lcosb_gmap_tree_node_addr_t	mapfrag_addr;
	
	uint8_t 	pl_bundle_state;
	uint8_t		multi_shard_id;

} lcosb_repo_ua_plb_registry_t;

/** Base funcs
 * 
*/

lcosb_repo_id_t lcosb_repo_createData(lcosb_repo_dtype_t dtype, const char* client_tag);

lcosb_sys_resp lcosb_repo_updateData(lcosb_repo_id_t repo_id, const char* request_tag, lcosb_repo_updtype update_type, const char* data);

void* lcosb_repo_getData(lcosb_repo_id_t repo_id, lcosb_repo_dtype_t dtype, const char* request_tag)


/** HL funcs
 * 
*/

//lcosb_repo_chkInMem;

lcosb_obj_base_t lcosb_repo_getNewObjBase(int obj_lvl);
void* lcosb_repo_registerObj(lcosb_repo_id_t repo_id, lcosb_repo_dtype_t dtype, void* in_ram_data_p);

// gmap will be stored static in gmap.h





#endif // LCOSB_REPOINTF_H
