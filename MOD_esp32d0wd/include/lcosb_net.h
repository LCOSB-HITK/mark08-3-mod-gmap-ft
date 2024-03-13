/***
	author: Lucius Pertis
	Complete project details at https://github.com/LCOSB-HITK/
***/

#ifndef LCOSB_NET_H
#define LCOSB_NET_H

#include "lcosb_config_dev.h"
#include "Arduino.h"
#include "esp_err.h"

//************************************************************
// this is a simple example that uses the painlessMesh library to
// connect to a another network and broadcast message from a webpage to the edges of the mesh network.
// This sketch can be extended further using all the abilities of the AsyncWebserver library (WS, events, ...)
// for more details
// https://gitlab.com/painlessMesh/painlessMesh/wikis/bridge-between-mesh-and-another-network
// for more details about my version
// https://gitlab.com/Assassynv__V/painlessMesh
// and for more details about the AsyncWebserver library
// https://github.com/me-no-dev/ESPAsyncWebServer
//************************************************************

#include "IPAddress.h"
#include "painlessMesh.h"

#include <ESPAsyncWebServer.h>

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

#define   STATION_SSID     "mySSID"
#define   STATION_PASSWORD "myPASSWORD"

#define HOSTNAME "HTTP_BRIDGE"

// Prototype
void receivedCallback( const uint32_t &from, const String &msg );
IPAddress getlocalIP();

painlessMesh  mesh;
AsyncWebServer server(80);

IPAddress myIP(0,0,0,0);
IPAddress myAPIP(0,0,0,0);

// mesh/map inf
int curr_phy_bound[4] = {0, 0, 0, 0};
unsigned long curr_mesh_time = 0;

void setupNet() {

	mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION ); 

	mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 6 );
	mesh.onReceive(&receivedCallback);

	mesh.stationManual(STATION_SSID, STATION_PASSWORD);
	mesh.setHostname(HOSTNAME);

	#if UNIT_IS ROOT > 0
	mesh.setRoot(true);
	#endif

	// This node and all other nodes should ideally know the mesh contains a root, so call this on all nodes
	mesh.setContainsRoot(true);

	myAPIP = IPAddress(mesh.getAPIP());
	Serial.println("My AP IP is " + myAPIP.toString());

	setupRequestHandlers();

	server.begin();
}

void loop() {
  mesh.update();
  if(myIP != getlocalIP()){
    myIP = getlocalIP();
    Serial.println("My IP is " + myIP.toString());
  }
}

void addCorsHeaders(AsyncWebServerResponse *response) {
	response->addHeader("Access-Control-Allow-Origin", "*");
	response->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
	response->addHeader("Access-Control-Allow-Headers", "*");
}

void setupRequestHandlers() {

	/** root
	 * 
	*/
	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
		
		// headers
		addCorsHeaders(request);

		//simple response
		request->send(200, "text/html", "<form>Text to Broadcast<br><input type='text' name='BROADCAST'><br><br><input type='submit' value='Submit'></form>");
		if (request->hasArg("BROADCAST")){
			String msg = request->arg("BROADCAST");
			mesh.sendBroadcast(msg);
		}
	});

	/** map_inf
	 * 
	*/
	server.on("/map_inf", HTTP_GET, [](AsyncWebServerRequest *request) {
		// Create JSON response object
		DynamicJsonDocument jsonResponse(200);

		// Populate JSON response object with current values
		JsonArray curr_phy_bound_array = jsonResponse.createNestedArray("curr_phy_bound");
		for (int i=0; i<4; i++)
			curr_phy_bound_array.add(curr_phy_bound[i]);

		// Populate JSON response object with current values
		//jsonResponse["curr_phy_bound"] = curr_phy_bound_str;
		jsonResponse["curr_mesh_time"] = curr_mesh_time;

		// Serialize JSON to string
		String responseString;
		serializeJson(jsonResponse, responseString);

		// headers
		addCorsHeaders(request);

		// Send JSON response
		request->send(200, "application/json", responseString);
  	});

	/** all robots stat
	 * 
	*/
	server.on("/robot_stat", HTTP_GET, [](AsyncWebServerRequest *request) {
		// Create JSON response object
		DynamicJsonDocument jsonResponse(1024); // Adjust size as needed

		// Create array to hold robot data
		JsonArray robotsArray = jsonResponse.createNestedArray("robots");

		//TODO: define robots in repo

		// Iterate through each robot and add its data to the array
		for (int i = 0; i < numRobots; i++) {
			JsonObject robotObj = robotsArray.createNestedObject();
			robotObj["unit_id"] = robots[i].unit_id;
			robotObj["x"] 		= robots[i].x;
			robotObj["y"] 		= robots[i].y;
			robotObj["theta"] 	= robots[i].theta;
			robotObj["v"] 		= robots[i].v;
			robotObj["omega"] 	= robots[i].omega;
			robotObj["rcurve"] 	= robots[i].rcurve;
			robotObj["d_l"] 	= robots[i].d_l;
			robotObj["d_r"] 	= robots[i].d_r;
			robotObj["ctime"] 	= robots[i].ctime;
		}

		// Serialize JSON to string
		String responseString;
		serializeJson(jsonResponse, responseString);

		// Add CORS and other headers
		addCorsHeaders(request);

		// Send JSON response with headers
		request->send(200, "application/json", responseString);
	});


}

void receivedCallback( const uint32_t &from, const String &msg ) {
  Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());
}

IPAddress getlocalIP() {
  return IPAddress(mesh.getStationIP());
}


#endif // LCOSB_NET_H
