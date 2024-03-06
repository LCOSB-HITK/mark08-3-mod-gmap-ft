/***
	author: Lucius Pertis
	Complete project details at https://github.com/LCOSB-HITK/
***/


#include "lcosb_log.h"
#include <string.h>
#include <stdlib.h>

// Circular buffer for logs
static const char* LOGS[BUFFER_SIZE];
static int logSizes[BUFFER_SIZE]; // Sizes of corresponding logs
static int head = 0;
static int tail = 0;

const char *TAGS[] = {
    "@sample_tag:"
};

void putLog(const char* logEntry, int tag) {


    // Check if the buffer is full
    if ((head + 1) % BUFFER_SIZE == tail) {
        // Buffer is full, move tail to the next position
        tail = (tail + 1) % BUFFER_SIZE;
    }

    // Free memory of the overwritten log if it exists
    if (LOGS[(head + 1) % BUFFER_SIZE] != NULL) {
        free((void*)LOGS[(head + 1) % BUFFER_SIZE]);
    }

    // Add log to the circular buffer
    LOGS[head] = strdup(logEntry);
    logSizes[head] = strlen(logEntry);
    head = (head + 1) % BUFFER_SIZE;
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
        const char* logEntry = LOGS[tail];
        int logSize = logSizes[tail];

        // Check if there's enough space in buff
        if (*size + logSize < buffSize) {
            // Copy log to the buffer
            memcpy(buff + *size, logEntry, logSize);
            *size += logSize;
            totalLogs++;

            // Move tail to the next position
            tail = (tail + 1) % BUFFER_SIZE;
        } else {
            // Stop if there's not enough space in buff
            break;
        }
    }

    // Return the number of packed logs
    return totalLogs;
}

