/***
	author: Lucius Pertis
	Complete project details at https://github.com/LCOSB-HITK/
***/

#ifndef LCOSB_LOG_H
#define LCOSB_LOG_H

#include "lcosb_config_dev.h"
#include "Arduino.h"
#include "esp_err.h"

extern static unsigned long lastLogTime;

#define BUFFER_SIZE 100

void putLog(const char* logEntry);
const char* getLog();
int getLogs(char* buff, int* size, int n, int buffSize);

#endif