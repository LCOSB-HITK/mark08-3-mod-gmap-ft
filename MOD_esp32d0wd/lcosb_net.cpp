/***
	author: Lucius Pertis
	Complete project details at https://github.com/LCOSB-HITK/
***/


#include "include/lcosb_net.h"

#include "include/lcosb_mesh_dataops.h"

#define   MESH_PREFIX     "LCOSB_MESH"
#define   MESH_PASSWORD   "pi=3.14159"
#define   MESH_PORT       5555

#define   STATION_SSID     "LCOSB_MESH_INF_ROOT"
#define   STATION_PASSWORD "pi=3.14159"

// Prototype
void meshReceivedCallback( const uint32_t &from, const String &msg );
IPAddress getlocalIP();

// mesh/map inf
int curr_phy_bound[4] = {0, 0, 0, 0};
unsigned long curr_mesh_time = 0;

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setupNet() {

	mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); 

	mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 6 );
	mesh.onReceive(&meshReceivedCallback);

	mesh.stationManual(STATION_SSID, STATION_PASSWORD);
	mesh.setHostname(HOSTNAME);

	#if UNIT_IS_ROOT > 0
	mesh.setRoot(true);
	#endif

	// This node and all other nodes should ideally know the mesh contains a root, so call this on all nodes
	mesh.setContainsRoot(true);

	myAPIP = IPAddress(mesh.getAPIP());
	Serial.println("My AP IP is " + myAPIP.toString());

	setupRequestHandlers();

	server.begin();
}

void meshLoopRoutine() {
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
		
		AsyncWebServerResponse *response = request->beginResponse(200, "text/html", "<form>Text to Broadcast<br><input type='text' name='BROADCAST'><br><br><input type='submit' value='Submit'></form>");
		
		addCorsHeaders(response);
		request->send(response);

		
		if (request->hasArg("BROADCAST")){
			String msg = request->arg("BROADCAST");
			mesh.sendBroadcast(msg, true);
		}
	});

	/** map_inf
	 * 
	*/
	server.on("/map_inf", HTTP_GET, [](AsyncWebServerRequest *request) {
		// Create JSON response object
		JsonDocument jsonResponse;

		// Populate JSON response object with current values
		JsonArray curr_phy_bound_array = jsonResponse["curr_phy_bound"].to<JsonArray>(); // jsonResponse.createNestedArray("curr_phy_bound") 
		for (int i=0; i<4; i++)
			curr_phy_bound_array.add(curr_phy_bound[i]);

		// Populate JSON response object with current values
		//jsonResponse["curr_phy_bound"] = curr_phy_bound_str;
		jsonResponse["curr_mesh_time"] = curr_mesh_time;

		// Serialize JSON to string
		String responseString;
		serializeJson(jsonResponse, responseString);

		AsyncWebServerResponse *response = request->beginResponse(200, "application/json", responseString);
		
		addCorsHeaders(response);

		// Send JSON response
		request->send(response);
  	});

	/** all robots stat
	 * 
	*/
	server.on("/robot_stat", HTTP_GET, [](AsyncWebServerRequest *request) {
		// Create JSON response object
		JsonDocument jsonResponse; // Adjust size as needed

		// Create array to hold robot data
		JsonArray robotsArray = jsonResponse.createNestedArray("robots");

		// Iterate through each robot in robots map and copy json_digest elem to jsonResponse
		for(const auto &node : mesh.getNodeList()) {
			if(ROBOT_STAT_REG.find(node.nodeId) > 0) {
				JsonObject robotObj = robotsArray.createNestedObject();
				
				// json_digest is a JsonDocument
				robotObj = ROBOT_STAT_REG[node.nodeId]->json_digest;
			}
		}

		// Serialize JSON to string
		String responseString;
		serializeJson(jsonResponse, responseString);

		AsyncWebServerResponse *response = request->beginResponse(200, "application/json", responseString);
		
		addCorsHeaders(response);
		request->send(response);

		#if UNIT_IS_ROOT > 0
		mesh.sendBroadcast(status_get_broadcast_str);
		#endif
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
		mesh.sendSingle(nodeId, postData);

		AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Command sent to node " + nodeIdStr);
		
		addCorsHeaders(response);

		// Send JSON response
		request->send(response);
	});

}

IPAddress getlocalIP() {
  return IPAddress(mesh.getStationIP());
}
