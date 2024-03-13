/***
	author: Lucius Pertis
	Complete project details at https://github.com/LCOSB-HITK/
***/


#include "include/lcosb_mesh_dataops.h"

#include "include/lcosb_echo.h"
#include "include/lcosb_motor.h"
#include "include/lcosb_lame.h"

#include "include/lcosb_log.h"
#include "lcosb_mesh_dataops.h"

unordered_map<int, robot_status_t> robots;

const String status_get_broadcast_str = "{\"type\":\"status\",\"method\":\"GET\"}";

int initMeshDataOps()
{
	// Initialize the robot status with self data
	String json_msg;
	createRobotStatus(mesh.getNodeId(), json_msg);
	Serial.println("Self status created");
	Serial.println(json_msg);
}



void meshReceivedCallback( const uint32_t &from, const String &msg ) {

    Serial.printf("Received from %u msg=%s\n", from, msg.c_str());

	// Parse the json command
	JsonDocument doc;
	DeserializationError error = deserializeJson(doc, msg);
	if (error) {
		Serial.print(F("deserializeJson() failed: "));
		Serial.println(error.c_str());
		return;
	}

	// get command type
	String type = doc["type"];
	String method = doc["method"];

	if (type == "status") {
		int req_unit_id = doc["unit_id"];
		int req_gtime = doc["gtime"];

		if (req_unit_id == 0)	req_unit_id = from;

		if (doc.containsKey("unit_id") == 0)
		int req_unit_id = doc["unit_id"];
		int req_gtime = doc["gtime"];

		if (method == "GET") {

			// send jsoned status to "from" node
			int pass = createRobotStatus(req_unit_id, NULL);

			if (pass == 0 && req_gtime < robots[req_unit_id].gtime
				&& req_unit_id != mesh.getNodeId()) {
				// create response
				String resp_json_msg;
				JsonDocument resp_doc;

				resp_doc["type"] = "status";
				resp_doc["method"] = "POST";
				resp_doc["unit_id"] = req_unit_id;
				resp_doc["json_digest"] = robots[req_unit_id].json_digest;

				serializeJson(resp_doc, resp_json_msg);

				mesh.sendSingle(from, resp_json_msg);
			}
			else {
				Serial.println("Status not sent");
			}

		} else if (method == "POST") {
			// update the status of the "from" node
			JsonDocument upd_json_digest = doc["json_digest"];

			int pass = updateRobotStatus(upd_json_digest);
			if (pass == 0)	Serial.println("Status updated");
			else 			Serial.println("Status not updated");
		}

	}
	else if (type == "ctrlMotor") {

		int l = doc["l"];
		int r = doc["r"];

		setMotorSpeed(l, r);
        updateMotorOutput();
	}
	else if (type == "moveRover") {

		int pow_value = doc["p"];
		int steer_value = doc["s"];

		steerRover(pow_value, steer_value);
	}
	else if (type == "undefined") {
		
	}
	else {
		Serial.println("Unknown command type");
	}
	

}

int updateRobotStatus(JsonDocument &upd_json_digest) {
	// chk if unit_id in the map
	int req_unit_id = upd_json_digest["unit_id"];

	if (robots.find(req_unit_id) == robots.end()) {
		Serial.println("req_unit_id not found");

		robot_status_t *robot = &robots[req_unit_id];
		robot->unit_id = req_unit_id;
		robot->gtime = upd_json_digest["ctime"] / 1000; // secs
		robot->json_digest = upd_json_digest;
	} 
	else if (req_unit_id != mesh.getNodeId()) {
		if (robots[req_unit_id].gtime > upd_json_digest["ctime"] / 1000) { // old status
			Serial.println("Old status");
			return -3;
		}
		
		robots[req_unit_id].json_digest = upd_json_digest;
		robots[req_unit_id].gtime = upd_json_digest["ctime"] / 1000; // secs
	} 
	else { // self match
		Serial.println("self match");
		return -1;
	}

	return 0;
}

int createRobotStatus(int unit_id, JsonDocument &resp_json_digest) {
	if (unit_id == mesh.getNodeId()) { // self status
		char digest[100];
		int sd_f = get_sys_digest(digest, 100);
		if(sd_f != -1) { // digest creation success

			// update status in robots map
			robot_status_t *robot = &robots[unit_id];
			robot->gtime = mesh.getNodeTime() / 1000 / 1000; // secs

			// Reverse cscanf snippet
			int x,y,theta,v,omega,rcurve,l,r,d_l,d_r,ctime;
			sscanf(msgbuff, "%d %d %d %d %d %d %d %d %d %d",
				&ctime, &x, &y, &theta, &v, &omega, &rcurve, &l, &r, &d_l, &d_r);


			json_digest["unit_id"] = mesh.getNodeId();
			json_digest["str_digest"] = digest;
			json_digest["x"] = x;
			json_digest["y"] = y;
			json_digest["theta"] = theta;
			json_digest["v"] = v;
			json_digest["omega"] = omega;
			json_digest["rcurve"] = rcurve;
			json_digest["l"] = l;
			json_digest["r"] = r;
			json_digest["d_l"] = d_l;
			json_digest["d_r"] = d_r;
			json_digest["ctime"] = mesh.getNodeTime() / 1000; // msecs

			if (resp_json_digest != NULL)
				resp_json_digest = json_digest; // copy to resp_json_digest
			return 0;
		}
		else return -1;
	}
	else if (robots.find(unit_id) != robots.end()) {
		if (resp_json_digest != NULL)
			resp_json_digest = json_digest; // copy to resp_json_digest
		return 0;
	}
	
	else if (robots.find(unit_id) == robots.end()) {
		return -2;
	}
    
    return 1;
}

int get_sys_digest(char* msgbuff, int size) {
	int curr_gpos[3], curr_gvel[3], curr_motor[2];

	getGPos(curr_gpos);
	getGVel(curr_gvel);

	lcosb_echo_t curr_echo;
	recordEcho(&curr_echo);

	curr_motor[0] = getMotorSpeed(0);
	curr_motor[1] = getMotorSpeed(1);

	int cw = snprintf(
				msgbuff, size,
				"%d %d %d %d %d %d %d %d %d %d %d\n\0",
				mesh.getNodeTime() / 1000,
				curr_gpos[0], curr_gpos[1], curr_gpos[2],
				curr_gvel[0], curr_gvel[1], curr_gvel[2],
				curr_motor[0], curr_motor[1],
				curr_echo.left, curr_echo.right
				);

	if (cw < 0 || cw >= size) {
		return -1;
	} else {
		return cw;
	}
}
