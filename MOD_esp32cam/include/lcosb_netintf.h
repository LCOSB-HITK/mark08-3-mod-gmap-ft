/***
	author: Lucius Pertis
	Complete project details at https://github.com/LCOSB-HITK/
***/

#ifndef LCOSB_NETINTF_H
#define LCOSB_NETINTF_H

#include "lcosb_config_dev.h"
#include "Arduino.h"
#include "esp_err.h"

#include "mark08_ft_httpd.h"

#define NET_MAX_UNITS 10

#define NET_ID_D0WD_OFFSET	0
#define NET_ID_CAM_OFFSET	2
#define NET_ID_SD_OFFSET	4

const char* NET_SSID = "LPn9";
const char* NET_PASS = "pi=3.14159";



// registry for unit id and ip address
typedef struct {
	uint8_t		unit_id;
	uint8_t		ip_addr_d0wd[4];
	uint8_t		ip_addr_cam[4];
} lcosb_unit_registry_t;


/** self hardcoded unit_id
 * 
 * unit_id is the unique identifier for each unit/esp chip
 * 
 * if unit_id if multiple of 5, then it is a d0wd unit
 * - unit_id % 5 == 0 => d0wd (in ram)
 * - unit_id % 5 == 1 => d0wd (in memcache)
 * - unit_id % 5 == 2 => cam (in ram)
 * - unit_id % 5 == 3 => cam (in memcache)
 * - unit_id % 5 == 4 => cam (in flash/sdcard)
 * 
 * unit_ids can range from 0b000000xx to 0b111111xx (the 6 msb bits) 
 * supporting around 64/5 ~ 12 units
 * the 2 lsb bits are reserved for quick bit validation
*/ 
uint8_t lcosb_net_self_unit_id;
const uint8_t NET_UNIT_TYPE = NET_ID_CAM_OFFSET;

// inference ip address
uint8_t lcosb_net_infserver_ip[4], lcosb_net_infserver_port = 80;


lcosb_unit_registry_t lcosb_net_unit_registry[NET_MAX_UNITS] = {{0,{{0}}, {{0}}}};


int lcosb_net_setSelfUnitId(uint8_t unit_id) {
	lcosb_net_self_unit_id = ((unit_id * 5 + NET_UNIT_TYPE) << 2) | 0b11;
	lcosb_net_unit_registry[0].unit_id = lcosb_net_self_unit_id;

	return 0;
}

int lcosb_net_setUnitIdIp(uint8_t id, uint8_t ip_addr_d0wd[4], uint8_t ip_addr_cam[4]) {
	// if id found, then update entry
	for (int i = 0; i < NET_MAX_UNITS; i++) {
		if (lcosb_net_unit_registry[i].unit_id == id) {
			for (int j = 0; j < 4; j++) {
				lcosb_net_unit_registry[i].ip_addr_d0wd[j] = ip_addr_d0wd[j];
				lcosb_net_unit_registry[i].ip_addr_cam[j] = ip_addr_cam[j];
			}
			return 0;
		}
	}

	// if id not found, then add new entry
	for (int i = 0; i < NET_MAX_UNITS; i++) {
		if (lcosb_net_unit_registry[i].unit_id == 0) {
			lcosb_net_unit_registry[i].unit_id = id;
			for (int j = 0; j < 4; j++) {
				lcosb_net_unit_registry[i].ip_addr_d0wd[j] = ip_addr_d0wd[j];
				lcosb_net_unit_registry[i].ip_addr_cam[j] = ip_addr_cam[j];
			}
			return 0;
		}
	}
}

int lcosb_net_setInfseverIp(uint8_t ip_addr[4], uint8_t port) {
	for (int i = 0; i < 4; i++) {
		lcosb_net_infserver_ip[i] = ip_addr[i];
	}
	lcosb_net_infserver_port = port;
	return 0;
}

int lcosb_net_testPing(uint8_t unit_id) {
	// if unit_id found, then ping
	for (int i = 0; i < NET_MAX_UNITS; i++) {
		if (lcosb_net_unit_registry[i].unit_id == unit_id) {
			// ping
			Serial.p
			int d0wd = pingIp(lcosb_net_unit_registry[i].ip_addr_d0wd);
			Serial.print("ping d0wd: "); Serial.println(d0wd);
			int cam = pingIp(lcosb_net_unit_registry[i].ip_addr_cam);
			Serial.print("ping cam: "); Serial.println(cam);

			if (d0wd == 0 && cam == 0) {
				return 0;
			} else {
				return 1;
			}
		}
	}
	return 2;
}


#endif // LCOSB_NETINTF_H
