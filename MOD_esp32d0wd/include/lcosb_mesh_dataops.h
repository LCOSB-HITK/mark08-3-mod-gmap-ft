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

#include "include/lcosb_echo.h"
#include "include/lcosb_motor.h"
#include "include/lcosb_lame.h"

#include "include/lcosb_log.h"

void receivedCallback( const uint32_t &from, const String &msg ) {
	Serial.printf("Received from %u msg=%s\n", from, msg.c_str());

	// Parse the json command
	StaticJsonDocument<200> doc;
	DeserializationError error = deserializeJson(doc, msg);
	if (error) {
		Serial.print(F("deserializeJson() failed: "));
		Serial.println(error.c_str());
		return;
	}

	// get command type
	String type = doc["type"];

	if (type == "status") {
		// force publish to the root node

	}
	else if (type == "ctrlMotor") {

		int l = doc["l"];
		int r = doc["r"];

		setMotorSpeed(l_m_v, r_m_v);
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

int status_handler() {
    char buff[100];
    int sd_f = get_sys_digest(buff, 100);
    
    if(sd_f==-1) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "digest error");
        return ESP_FAIL;
    }

    httpd_resp_send(req, buff, strlen(buff));
    return ESP_OK;
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
				"%lu %d %d %d %d %d %d %d %d %d %d\n\0",
				curr_echo.ctime,
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

#endif // LCOSB_MESH_DATAOPS_H
