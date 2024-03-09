/***
	author: Lucius Pertis
	Complete project details at https://github.com/LCOSB-HITK/
***/


#include "include/lcosb_log.h"
#include <string.h>
#include <stdlib.h>

#include <Arduino.h>

// Circular buffer for logs
static const char* LOGS[BUFFER_SIZE];
static int logSizes[BUFFER_SIZE]; // Sizes of corresponding logs
static int head = 0;
static int tail = 0;


const char *TAGS[] = {
    "@sample_tag: ",
    "@echo: ",
    "@lame: ",
    "@task: ",
};

void putLog(const char* logEntry, int tag) {

    // Check if the buffer is full
    if ((head + 1) % BUFFER_SIZE == tail) {
        // Buffer is full, move tail to the next position
        tail = (tail + 1) % BUFFER_SIZE;
        Serial.println("Buffer is full. Moving tail to the next position.");
    }

    // Free memory of the overwritten log if it exists
    if (LOGS[(head + 1) % BUFFER_SIZE] != NULL) {
        free((void*)LOGS[(head + 1) % BUFFER_SIZE]);
        Serial.println("Freeing memory of the overwritten log.");
    }

    Serial.println("Allocating Memory for the new log.");

    // add tag to the log
    char* taggedLog = (char*)malloc(strlen(logEntry) + strlen(TAGS[tag]) + 1);
    
    Serial.println("STRCPY on new log mem .");
    strcpy(taggedLog, TAGS[tag]);
    strcat(taggedLog, logEntry);

    // Add log to the circular buffer
    LOGS[head] = strdup(logEntry);
    logSizes[head] = strlen(logEntry);
    head = (head + 1) % BUFFER_SIZE;

    Serial.println("Log added to the circular buffer.");
}

const char* getLog() {
    // Check if there are logs to retrieve
    if (head != tail) {
        // Get log from the circular buffer
        const char* logEntry = LOGS[tail];
        tail = (tail + 1) % BUFFER_SIZE;
        return logEntry;
    }
    return NULL;  // Return NULL if the buffer is empty
}

int getLogs(char* buff, int* size, int n, int buffSize) {
    int totalLogs = 0;

    // Pack logs until the desired number is reached
    while (head != tail && totalLogs < n) {
        Serial.print("head at: ");
        Serial.println(head);
        Serial.print("tail at: ");
        Serial.println(tail);
        const char* logEntry = LOGS[tail];
        int logSize = logSizes[tail];
        Serial.print("curr log size: ");
        Serial.println(logSize);

        // Check if there's enough space in buff
        if (*size + logSize < buffSize) {

            Serial.println("Copying log to the buffer using MEMCPY.");
            // Copy log to the buffer
            memcpy(buff + *size, logEntry, logSize);
            *size += logSize;
            totalLogs++;

            Serial.print("Total logs packed: ");
            Serial.println(totalLogs);

            // Move tail to the next position
            tail = (tail + 1) % BUFFER_SIZE;
        } else {
            // Stop if there's not enough space in buff
            Serial.println("Not enough space in the output buffer.");
            break;
        }

        Serial.print("upd tail: "); Serial.println(tail);
    }

    // Return the number of packed logs
    return totalLogs;
}

