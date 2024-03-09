/***
	author: Lucius Pertis
	Complete project details at https://github.com/LCOSB-HITK/
***/


#include "lcosb_repointf.h"

static lcosb_repo_dataobj_registry_t REPO_OBJ_REG[96];
static uint8_t REPO_OBJ_REG_SLOTS_STAT[96] = {0};

static lcosb_repo_ua_plb_registry_t REPO_UA_PLB[96];
static uint8_t REPO_UA_PLB_STAT[96] = {0};

// gmap will be stored static in gmap.h
