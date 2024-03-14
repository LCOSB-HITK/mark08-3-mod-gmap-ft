/***
	author: Lucius Pertis
	Complete project details at https://github.com/LCOSB-HITK/
***/


#include "include/lcosb_mesh_dataops.h"

#include "include/lcosb_echo.h"
#include "include/lcosb_motor.h"
#include "include/lcosb_lame.h"

#include "include/lcosb_log.h"

RobotStatReg ROBOT_STAT_REG = RobotStatReg();
const String status_get_broadcast_str = "{\"type\":\"status\",\"method\":\"GET\"}";

int initMeshDataOps()
{
	// Initialize the robot status with self data
	createRobotStatus(LCOSB_MESH.getNodeId());
	Serial.println("Self status created");
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

		if (method == "GET") {

			// send jsoned status to "from" node
			int pass = createRobotStatus(req_unit_id);

			if (pass == 0 && req_gtime < ROBOT_STAT_REG[req_unit_id].gtime
				&& req_unit_id != LCOSB_MESH.getNodeId()) {
				// create response
				String resp_json_msg;
				JsonDocument resp_doc;

				resp_doc["type"] = "status";
				resp_doc["method"] = "POST";
				resp_doc["unit_id"] = req_unit_id;
				resp_doc["json_digest"] =ROBOT_STAT_REG[req_unit_id].json_digest;

				serializeJson(resp_doc, resp_json_msg);

				LCOSB_MESH.sendSingle(from, resp_json_msg);
			}
			else {
				Serial.println("Status not sent");
			}

		} else if (method == "POST") {
			// update the status of the "from" node
			JsonObject upd_json_digest = doc["json_digest"];

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

int updateRobotStatus(JsonObject &upd_json_digest) {
	// chk if unit_id in the map
	int req_unit_id = upd_json_digest["unit_id"];

	if (ROBOT_STAT_REG.find(req_unit_id) == -1) {
		Serial.println("req_unit_id not found");

		robot_status_t *robot = &ROBOT_STAT_REG[req_unit_id];
		robot->unit_id = req_unit_id;
		robot->gtime = ((int) upd_json_digest["ctime"]) / 1000; // secs
		robot->json_digest = upd_json_digest;
	} 
	else if (req_unit_id != LCOSB_MESH.getNodeId()) {
		if (ROBOT_STAT_REG[req_unit_id].gtime > ((int) upd_json_digest["ctime"]) / 1000) { // old status
			Serial.println("Old status");
			return -3;
		}
		
		ROBOT_STAT_REG[req_unit_id].json_digest = upd_json_digest;
		ROBOT_STAT_REG[req_unit_id].gtime = ((int) upd_json_digest["ctime"]) / 1000; // secs
	} 
	else { // self match
		Serial.println("self match");
		return -1;
	}

	return 0;
}

int createRobotStatus(int unit_id) {
    
	if (unit_id == LCOSB_MESH.getNodeId()) { // self status
		char digest[100];
		int sd_f = get_sys_digest(digest, 100);
		if(sd_f != -1) { // digest creation success
            // Reverse cscanf snippet
			int x,y,theta,v,omega,rcurve,l,r,d_l,d_r,ctime;
			sscanf(digest, "%d %d %d %d %d %d %d %d %d %d",
				&ctime, &x, &y, &theta, &v, &omega, &rcurve, &l, &r, &d_l, &d_r);
            
			// update status in robots map
			robot_status_t *robot = &ROBOT_STAT_REG[unit_id];
			robot->gtime = LCOSB_MESH.getNodeTime() / 1000 / 1000; // secs

			robot->json_digest["unit_id"] = LCOSB_MESH.getNodeId();
			robot->json_digest["str_digest"] = digest;
			robot->json_digest["x"] = x;
			robot->json_digest["y"] = y;
			robot->json_digest["theta"] = theta;
			robot->json_digest["v"] = v;
			robot->json_digest["omega"] = omega;
			robot->json_digest["rcurve"] = rcurve;
			robot->json_digest["l"] = l;
			robot->json_digest["r"] = r;
			robot->json_digest["d_l"] = d_l;
			robot->json_digest["d_r"] = d_r;
			robot->json_digest["ctime"] = (int) LCOSB_MESH.getNodeTime() / 1000; // msecs

			// if (resp_json_digest != NULL)
			// 	resp_json_digest = robot->json_digest; // copy to resp_json_digest
			return 0;
		}
		else return -1;
	}
	else if (ROBOT_STAT_REG.find(unit_id) != -1) {
        // if (resp_json_digest != NULL)
		// 	resp_json_digest = robot->json_digest; // copy to resp_json_digest
		return 0;
	}
	
	else if (ROBOT_STAT_REG.find(unit_id) == -1) {
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
				LCOSB_MESH.getNodeTime() / 1000,
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
