/***
	author: Lucius Pertis
	Complete project details at https://github.com/LCOSB-HITK/
***/

#ifndef TASKS_BASIC_H
#define TASKS_BASIC_H

#include "lcosb_config_dev.h"
#include <Arduino.h>
#include "esp_err.h"


#include "lcosb_echo.h"
#include "freertos/FreeRTOS.h"

#include "lcosb_lame.h"

// due to mesh we need to use Scheduler from TaskScheduler@3.7.0
#include <TaskScheduler.h>
extern Scheduler LCOSB_TASK_SCHEDULER;

// time period in secs
void startEchoRecordTimed(int count, int time_period); 

// single convertion
void convertEcho2PL();

void processEchoData(lcosb_echo_bundle_t* echo_b, int mid_dist, lcosb_echo_pl_t* pl, float slope, int start, int end, double max_x_per_size, double max_y_per_size, int* unit_err);

float calcEchoSlope(int* dists, int len, int* start, int* end, float* e_slope, int* mid_dist);

#endif // TASKS_BASIC_H
