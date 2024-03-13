/***
	author: Lucius Pertis
	Complete project details at https://github.com/LCOSB-HITK/
***/

#ifndef LCOSB_NET_H
#define LCOSB_NET_H

#include "lcosb_config_dev.h"
#include "Arduino.h"
#include "esp_err.h"

#include "IPAddress.h"
#include "painlessMesh.h"
#include <ESPAsyncWebServer.h>

#define HOSTNAME "LCSOB_MESH_HTTP_BRIDGE"


painlessMesh  mesh;
AsyncWebServer server(80);

IPAddress myIP(0,0,0,0);
IPAddress myAPIP(0,0,0,0);

void setupNet();
void meshLoopRoutine();
void addCorsHeaders(AsyncWebServerResponse *response);
void setupRequestHandlers();
IPAddress getlocalIP();

#endif // LCOSB_NET_H
