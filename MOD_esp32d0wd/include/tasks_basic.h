/***
	author: Lucius Pertis
	Complete project details at https://github.com/LCOSB-HITK/
***/

#ifndef TASKS_BASIC_H
#define TASKS_BASIC_H

#include "lcosb_config_dev.h"
#include "Arduino.h"
#include "esp_err.h"

// time period in secs
void startEchoRecordTimed(int count, int time_period); 

// single convertion
void convertEcho2PL();

#endif // TASKS_BASIC_H
