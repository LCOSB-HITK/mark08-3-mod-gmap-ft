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

#include "lcosb_mesh_dataops.h"

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

	/** robots specific
	 * 
	*/
	server.on("/node-fwd", HTTP_ANY, [](AsyncWebServerRequest *request) {
		// Extract node ID from query parameter
		String nodeIdStr = request->getParam("nid")->value();
		int nodeId = nodeIdStr.toInt(); // Convert node ID to integer

		// Extract remaining URL (without '/node-fwd')
		String remainingUrl = request->url();
		remainingUrl.remove(0, 9); // Remove '/node-fwd' (assuming 9 characters)

		// Extract post data
		String postData = "";
		if (request->hasParam("plain")) {
			postData = request->getParam("plain")->value();
		}

		// Send remaining URL and post data as string to the specified node using sendSingle
		mesh.sendSingle(nodeId, remainingUrl + "?" + postData);

		// Send a response indicating success
		request->send(200, "text/plain", "Command sent to node " + nodeIdStr);
	});

}

void receivedCallback( const uint32_t &from, const String &msg ) {
  Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());
}

IPAddress getlocalIP() {
  return IPAddress(mesh.getStationIP());
}


#endif // LCOSB_NET_H
