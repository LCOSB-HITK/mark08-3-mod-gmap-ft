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

	JsonObject json_digest;
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
	};

	// find method
	int find(int unit_id) {
		for (int i = 0; i < MAX_ROBOTS; i++) {
			if (robot_array[i].unit_id == unit_id) {
				return unit_id;
			}
		}

		return -1;
	};

};

RobotStatReg ROBOT_STAT_REG = RobotStatReg();

const String status_get_broadcast_str = "{\"type\":\"status\",\"method\":\"GET\"}";

int initMeshDataOps();

void meshReceivedCallback( const uint32_t &from, const String &msg );
int get_sys_digest(char* msgbuff, int size);

int createRobotStatus(int unit_id);
int updateRobotStatus(JsonObject &upd_json_digest);


#endif // LCOSB_MESH_DATAOPS_H
