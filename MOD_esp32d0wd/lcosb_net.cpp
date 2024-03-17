/***
	author: Lucius Pertis
	Complete project details at https://github.com/LCOSB-HITK/
***/


#include "include/lcosb_net.h"
#include "painlessMesh.h"

#include "include/lcosb_mesh_dataops.h"


#define   MESH_PREFIX     "LCOSB_MESH"
#define   MESH_PASSWORD   "pi=3.14159"
#define   MESH_PORT       5555

#define   STATION_SSID     "Wifi16"
#define   STATION_PASSWORD "1604@2022"

painlessMesh  LCOSB_MESH;
AsyncWebServer server(80);

IPAddress myIP(0,0,0,0);
IPAddress myAPIP(0,0,0,0);


// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain

Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

void sendMessage() {
  String msg = "Hello from node ";
  msg += LCOSB_MESH.getNodeId();
  LCOSB_MESH.sendBroadcast( msg );
  taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 5 ));
  Serial.println("My IP is " + myIP.toString());
  Serial.println("My APIP is " + myAPIP.toString());
  Serial.println("My Station IP is " + LCOSB_MESH.getStationIP().toString());
}


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
    Serial.printf("Adjusted time %u. Offset = %d\n", LCOSB_MESH.getNodeTime(),offset);
}

Scheduler userScheduler; // to control your personal task

void setupNet() {

	LCOSB_MESH.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); 

	LCOSB_MESH.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA, 6 );
	
	LCOSB_MESH.onReceive(&meshReceivedCallback);
	LCOSB_MESH.onNewConnection(&newConnectionCallback);
	LCOSB_MESH.onChangedConnections(&changedConnectionCallback);
	LCOSB_MESH.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

	//LCOSB_MESH.stationManual(STATION_SSID, STATION_PASSWORD);
	//LCOSB_MESH.setHostname(HOSTNAME);
    //LCOSB_MESH.initStation();


	#if UNIT_IS_ROOT > 0
	LCOSB_MESH.setRoot(true);
	Serial.print(">> net :: @setup I am the root :");
	Serial.println(LCOSB_MESH.getNodeId());
	#endif

	// This node and all other nodes should ideally know the LCOSB_MESH contains a root, so call this on all nodes
	LCOSB_MESH.setContainsRoot(true);

	myAPIP = IPAddress(LCOSB_MESH.getAPIP());
	Serial.println("My AP IP is " + myAPIP.toString());

	setupRequestHandlers();
	server.begin();


    userScheduler.addTask( taskSendMessage );
    taskSendMessage.enable();
}

void meshLoopRoutine() {
	LCOSB_MESH.update();
	if(myIP != getlocalIP()){
		myIP = getlocalIP();
		Serial.println("My IP is " + myIP.toString());
	}

    //LCOSB_MESH.sendBroadcast("hello nigga");
  
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
			LCOSB_MESH.sendBroadcast(msg, true);
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
		JsonArray robotsArray = jsonResponse["robots"].to<JsonArray>();
        
        int keys[12];
		ROBOT_STAT_REG.getKeys(keys);

		#if LCOSB_DEBUG_LVL > LCOSB_VERBOSE	
			Serial.print(">> net :: @/robot_stat-handler ROBOT_STAT_REG keys: [ ");
			for(int i=0; i<12; i++) Serial.printf("%d, ", keys[i]);
			Serial.println(" ]");
		#endif

		for(int i=0; i<12; i++) {
            if(keys[i]==0) continue;

            int nodeId = keys[i];
            
			#if LCOSB_DEBUG_LVL > LCOSB_VERBOSE
			Serial.print(">> net :: @/robot_stat-handler nodeIds :");
			Serial.println(nodeId);
			#endif

			if(ROBOT_STAT_REG.find(nodeId) > 0) {
				#if LCOSB_DEBUG_LVL > LCOSB_VERBOSE
				Serial.print(">> net :: @/robot_stat-handler node found in ROBOT_STAT_REG");

				//ROBOT_STAT_REG.print(nodeId);
				#endif
				//JsonObject robotObj = robotsArray.createNestedObject();
				
				robotsArray.add(ROBOT_STAT_REG[nodeId].str_digest);
			}
		}

		// Serialize JSON to string
		String responseString;
		serializeJson(jsonResponse, responseString);

		AsyncWebServerResponse *response = request->beginResponse(200, "application/json", responseString);
		
		addCorsHeaders(response);
		request->send(response);

		#if UNIT_IS_ROOT > 0
		LCOSB_MESH.sendBroadcast(status_get_broadcast_str);
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
		LCOSB_MESH.sendSingle(nodeId, postData);

		AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Command sent to node " + nodeIdStr);
		
		addCorsHeaders(response);

		// Send JSON response
		request->send(response);
	});

}

IPAddress getlocalIP() {
  return IPAddress(LCOSB_MESH.getStationIP());
}
