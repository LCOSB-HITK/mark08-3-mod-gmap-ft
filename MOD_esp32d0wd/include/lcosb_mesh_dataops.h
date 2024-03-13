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


void meshReceivedCallback( const uint32_t &from, const String &msg );
int status_handler();
int get_sys_digest(char* msgbuff, int size);
#endif // LCOSB_MESH_DATAOPS_H
