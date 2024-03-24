/***
	author: Lucius Pertis
	Complete project details at https://github.com/LCOSB-HITK/
***/

#ifndef LCOSB_MESH_DATAOPS_H
#define LCOSB_MESH_DATAOPS_H

#include "lcosb_config_dev.h"
#include "Arduino.h"
#include "esp_err.h"


#include "IPAddress.h"
#include "painlessMesh.h"
#include <ESPAsyncWebServer.h>

#include "lcosb_net.h"

typedef struct {
	int	unit_id;
	int	gtime;

	String str_digest;
} robot_status_t;

#define MAX_ROBOTS 12
class RobotStatReg {
	robot_status_t robot_array[MAX_ROBOTS];
public:
	RobotStatReg() {
		for (int i = 0; i < MAX_ROBOTS; i++) {
			robot_array[i].unit_id = 0;
		}
	}
	
	// operator[] overload
	robot_status_t& operator[](int unit_id) {
		for (int i = 0; i < MAX_ROBOTS; i++) {
			if (robot_array[i].unit_id == unit_id) {
				return robot_array[i];
			}
		}

		// else return a 0 robot status object
		for (int i = 0; i < MAX_ROBOTS; i++) {
			if (robot_array[i].unit_id == 0) {
				robot_array[i].unit_id = unit_id;
				return robot_array[i];
			}
		}
		
		// if nothing available, return the last one
		robot_array[MAX_ROBOTS - 1].unit_id = unit_id;
		return robot_array[MAX_ROBOTS - 1];
	}

	// find method
	int find(int unit_id) {
		for (int i = 0; i < MAX_ROBOTS; i++) {
			if (robot_array[i].unit_id == unit_id) {
				return unit_id;
			}
		}

		return -1;
	}

	void getKeys(int *keys) {
		for(int i=0; i<MAX_ROBOTS; i++)
			keys[i] = robot_array[i].unit_id;
		
	}

	void toJStr(int unit_id, char* jstr_buff, int buffsize) {
		robot_status_t* rob = &robot_array[0];
		for (int i = 0; i < MAX_ROBOTS; i++) {
			if (robot_array[i].unit_id == unit_id) {
				rob = &robot_array[i];
				break;
			}
		}

		snprintf(jstr_buff, buffsize, "{\"unit_id\":%d,\"gtime\":%d,\"str_digest\":%s}", rob->unit_id, rob->gtime, rob->str_digest);
	}
};

extern RobotStatReg ROBOT_STAT_REG;
extern const String status_get_broadcast_str;

/** Status and meta data for gmap/logical-object transfer
 * 
*/
enum OBJ_REQ_TYPE {
	NULL_REQ,
	
	GET,
	POST,
	PUT,
	DELETE,

	mdo_CREATE,
	mdo_READ,
	mdo_UPDATE,
	mdo_DELETE,
	
	mdo_WRITE_EXCLUSIVE,	
};

int initMeshDataOps();
void meshReceivedCallback( const uint32_t &from, const String &msg );
int get_sys_digest(char* msgbuff, int size);

int createRobotStatus(int unit_id);
int updateRobotStatus(JsonDocument &upd_str_digest);


#endif // LCOSB_MESH_DATAOPS_H
