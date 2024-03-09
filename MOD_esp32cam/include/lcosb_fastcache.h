/***
	authors: Lucius Pertis
	Complete project details at https://github.com/LCOSB-HITK/
***/

#ifndef LCOSB_FASTCACHE_H
#define LCOSB_FASTCACHE_H

#include "lcosb_config_dev.h"
#include "Arduino.h"
#include "esp_err.h"

#include "lcosb_repo.h"

struct cache_entry {
	int key;
	int tag;
	int t2l; // ethier request t2l or cache data t2l
	void *dataobj; // if state is invalid then dataobj is NULL (for GET)
	int state;

	lcosb_repo_dtype_t dtype;
	int data_count;
};


struct cache_file_entry {
	uint16_t key;
	uint8_t tag;
	int t2l;
	
	char* cachefname;
	int file_offset;

	int state;
	lcosb_repo_dtype_t dtype;
};

int PUT


int lcosb_fc_selfCachePUT(int key, int tag, int t2l, void *dataobj, lcosb_repo_dtype_t dtype);
int lcosb_fc_selfCacheGET(int key, int tag, int t2l, void *dataobj, lcosb_repo_dtype_t dtype);
int lcosb_fc_selfCacheDEL(int key, int tag);

int lcosb_fc_repoCachePUT(int key, int tag, int t2l, int broadcast_type, void *dataobj, lcosb_repo_dtype_t dtype);
int lcosb_fc_repoCacheGET(int key, int tag, int t2l, int broadcast_type, void *dataobj, lcosb_repo_dtype_t dtype);




#endif // LCOSB_FASTCACHE_H
