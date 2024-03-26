/***
	author: Lucius Pertis
	Complete project details at https://github.com/LCOSB-HITK/
***/


#include "lcosb_repo.h"

// lock with GMAP


typedef struct {
	int obj_id;
	int obj_type;
	struct {
		int node_id;
		int upd_gtime;
	} scr[4];
	uint8_t latest_scr;
} lcosb_obj_cache_t;

class lcosb_repo_object_cache_register_queue {

	typedef struct obj_cache_reg_qnode {
		lcosb_obj_cache_t obj;
		uint8_t LRU_stat;
		struct obj_cache_reg_qnode* next;
	} obj_cache_reg_qnode_t;

	obj_cache_reg_qnode_t* HEAD;
	obj_cache_reg_qnode_t* TAIL;
	int count;

	obj_cache_reg_qnode_t* getObj(int obj_id) {
		obj_cache_reg_qnode_t* temp = HEAD;
		while (temp != NULL) {
			if (temp->obj.obj_id == obj_id) {
				return temp;
			}
			temp = temp->next;
		}
		return NULL;
	}

public:

	lcosb_repo_object_cache_register_queue() {
		HEAD = NULL;
		TAIL = NULL;
		count = 0;
	}

	/**
	 * @brief Get the Obj Scr Node ID
	 * 
	 * @param obj_id
	 * @param dirtiness
	 * @param earliest_gtime
	 * @return int node_id of the obj holder; -1 if no suitable scr found; 0 if obj not found
	*/
	int getObjScr(int obj_id, uint8_t dirtiness, int earliest_gtime) {
		obj_cache_reg_qnode_t* temp = this->getObj(obj_id);

		if (temp == NULL)
			return 0;
		else {
			temp->LRU_stat ++;

			// iterate the scr, find the dirtiest scr after earliest_gtime
			const uint8_t tscr = temp->latest_scr;
			dirtiness = min(dirtiness, 3);

			// dirtiest first
			for (int d = dirtiness; d >= 0; d--) {
				if (temp->obj.scr[(tscr + d)%4].upd_gtime > earliest_gtime) {
					return temp->obj.scr[(tscr + d)%4][0];	// return node_id
				}
			}

			// if no suitable scr found, return err
			return -1;
		}
	}

	int getObjScrLatest(int obj_id) {
		obj_cache_reg_qnode_t* temp = this->getObj(obj_id);

		if (temp == NULL)
			return 0;
		else {
			temp->LRU_stat ++;
			return temp->obj.scr[temp->latest_scr][0];
		}
	}

	void push(int obj_id, int obj_type, int node_id, int upd_gtime) {
		// if obj_id already exists, update the scr
		obj_cache_reg_qnode_t* temp = this->getObj(obj_id);

		if (temp != NULL) {
			temp->LRU_stat += 1;
			temp->obj.scr[temp->latest_scr+1].node_id = node_id;
			temp->obj.scr[temp->latest_scr+1].upd_gtime = upd_gtime;
			temp->latest_scr = (temp->latest_scr + 1) % 4;
			return;
		}

		// else create a new obj
		obj_cache_reg_qnode_t* new_obj = (obj_cache_reg_qnode_t*)malloc(sizeof(obj_cache_reg_qnode_t));
		
		new_obj->obj.obj_id = obj_id;
		new_obj->obj.scr[0].node_id = node_id;
		new_obj->obj.scr[0].upd_gtime = upd_gtime;
		for (int i = 1; i < 4; i++) {
			new_obj->obj.scr[i].node_id = -1;
			new_obj->obj.scr[i].upd_gtime = -1;		// int or uint?
		}
		new_obj->latest_scr = 0;

		new_obj->LRU_stat = 0;
		new_obj->next = NULL;

		// keeping it simple, no LRU eviction (LRU logic can be added later)
		if (HEAD == NULL) {
			HEAD = new_obj;
			TAIL = new_obj;
		} else {
			TAIL->next = new_obj;
			TAIL = new_obj;
		}

		this->count++;
	}

	lcosb_obj_cache_t* remove(int obj_id) {
		obj_cache_reg_qnode_t* prev = HEAD;
		obj_cache_reg_qnode_t* temp = NULL;

		if (HEAD->obj.obj_id == obj_id) {
			temp = HEAD;
			HEAD = HEAD->next;
			count--;
			return temp;
		}

		while (prev->next != NULL) {
			if (prev->next->obj.obj_id == obj_id) {
				temp = prev->next;
				prev->next = prev->next->next;
				count--;
				return temp;
			}
			prev = prev->next;
		}
		return NULL;
	}

	lcosb_obj_cache_t* pop(int obj_id) {
		obj_cache_reg_qnode_t* prev = HEAD;

		while (prev != NULL) {
			if (prev->obj.obj_id == obj_id) {
				return prev;
			}
			prev = prev->next;
		}
		return NULL;
	}

	int size() {
		return count;
	}
};

lcosb_repo_object_cache_register_queue OBJ_CACHE_REG;

