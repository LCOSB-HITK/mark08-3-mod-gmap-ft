/***
	author: Lucius Pertis
	Complete project details at https://github.com/LCOSB-HITK/
***/

#ifndef LCOSB_DBREPO_H
#define LCOSB_DBREPO_H

#include "lcosb_config_dev.h"
#include "Arduino.h"
#include "esp_err.h"


lcosb_repo_id lcosb_dbStore(lcosb_repo_dtype dtype, const char* data, const char* client_tag);
lcosb_repo_id lcosb_dbDelete(lcosb_repo_id repo_id, const char* client_tag);

typedef struct lcosb_db_data_chunk {
	lcosb_repo_dtype dtype;
	char chunk_num;
	int tot_size;

	const char* data;
} lcosb_db_data_chunk_t;

typedef struct lcosb_db_data {
	lcosb_repo_id repo_id;
	char chunk_count;

	lcosb_db_data_chunk_t* chunks;
} lcosb_db_data_t;

UA_PL

lcosb_db_data_chunk_t lcosb_dbLoad(lcosb_repo_id repo_id);



#endif // LCOSB_DBREPO_H
