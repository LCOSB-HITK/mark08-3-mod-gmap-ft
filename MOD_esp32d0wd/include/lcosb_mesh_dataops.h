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

typedef struct {
	int	unit_id;
	int	gtime;

	JsonDocument json_digest;
} robot_status_t;

extern unordered_map<int, robot_status_t> robots[12];
extern const String status_get_broadcast_str;

int initMeshDataOps();

void meshReceivedCallback( const uint32_t &from, const String &msg );
int status_handler();
int get_sys_digest(char* msgbuff, int size);
#endif // LCOSB_MESH_DATAOPS_H
