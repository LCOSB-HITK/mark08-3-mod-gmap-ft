/***
	author: Lucius Pertis
	Complete project details at https://github.com/LCOSB-HITK/
***/

#include <Arduino.h>

#include "include/tasks_basic.h"

// defined in lcosb_lame.h
// double fast_atan(double x) {
//     return x * (M_PI / 4.0) - x * (fABS(x) - 1) * (0.2447 + 0.0663 * fABS(x));
// }

// time period in msecs
void startEchoRecordTimed(int count, int time_period) {
	for(int i=0; i<count; i++) {
		recordEcho(NULL);
		vTaskDelay(time_period/portTICK_PERIOD_MS);
	}
}

// single convertion
void convertEcho2PL() {
	lcosb_echo_bundle_t* echo_b = (lcosb_echo_bundle_t*) malloc(sizeof(lcosb_echo_bundle_t));

	if (echo_b == NULL) {
		return;
	}

	if(echoBuffRead(echo_b) != 0) {
		return;
	};

	// start convertion compute
	lcosb_echo_pl_t* pl = (lcosb_echo_pl_t*) malloc(sizeof(lcosb_echo_pl_t));

	if(pl == NULL) {
		return;
	};

	int s1,e1,s2,e2;
	float e_slope;
	int unit_err[3];
	double max_x_per_size = echo_b->unit_pos.gvel[0]*cos((double)echo_b->unit_pos.gpos[2]/1000)*(echo_b->e_gtime - echo_b->s_gtime)/echo_b->size;
	double max_y_per_size = echo_b->unit_pos.gvel[0]*sin((double)echo_b->unit_pos.gpos[2]/1000)*(echo_b->e_gtime - echo_b->s_gtime)/echo_b->size;


	float slope_l = calcEchoSlope(echo_b->l, echo_b->size, &s1, &e1, &e_slope)/echo_b->unit_pos.gvel[0]; // mm/mm => rad
	slope_l = fast_atan(slope_l);
	
	getGPosErr(unit_err); unit_err[2] += e_slope*1000;

	// create and store pl for l
	processEchoData(echo_b, pl, slope_l, s1, e1, max_x_per_size, max_y_per_size, unit_err);
	if(echo_writePLStore(pl) != 0) {
		if (pl != NULL) free(pl);
		return; // store failed
	}
	
	// create and store pl for r
	pl = (lcosb_echo_pl_t*) malloc(sizeof(lcosb_echo_pl_t));
	if(pl == NULL) {
		return;
	};

	float slope_r = calcEchoSlope(echo_b->r, echo_b->size, &s2, &e2, &e_slope)/echo_b->unit_pos.gvel[0]; // mm/mm => rad
	slope_r = fast_atan(slope_r);

	getGPosErr(unit_err); unit_err[2] += e_slope*1000;

	processEchoData(echo_b, pl, slope_r, s2, e2, max_x_per_size, max_y_per_size, unit_err);
	if(echo_writePLStore(pl) != 0) {
		if (pl != NULL) free(pl);
		return; // store failed
	}

}

/**
 * Processes the echo data and stores the processed data in the pl record.
 * 
 * @param echo_b The echo bundle containing the echo data.
 * @param pl The pl record to store the processed data.
 * @param slope The slope of the echo data.
 * @param start The index of the start point.
 * @param end The index of the end point.
 * @param max_x_per_size The maximum x value per size.
 * @param max_y_per_size The maximum y value per size.
 * @param unit_err The error in the unit position.
 * 
 * 
*/
void processEchoData(lcosb_echo_bundle_t* echo_b, lcosb_echo_pl_t* pl, float slope, int start, int end, double max_x_per_size, double max_y_per_size, int* unit_err) {
	if (fABS(slope) < 15/180*M_PI) {
		pl->com[2] = (echo_b->unit_pos.gpos[2] + slope)*1000; // in mradians

		pl->com[0] = echo_b->unit_pos.gpos[0] + max_x_per_size*(start+end)/2;
		pl->com[1] = echo_b->unit_pos.gpos[1] + max_y_per_size*(start+end)/2;

		pl->glen = echo_b->unit_pos.gpos[0]*(echo_b->e_gtime - echo_b->s_gtime)
					*(end-start)/echo_b->size
					/cos(slope);
		
		pl->gtime = (echo_b->s_gtime + echo_b->e_gtime)/2;

		pl->acc_err[0] = echo_b->unit_pos.gpos[0]/2 + unit_err[0];
		pl->acc_err[1] = echo_b->unit_pos.gpos[1]/2 + unit_err[1];
		pl->acc_err[2] = unit_err[2];
	}
}

/**
 * Calculates the slope of the echo data points and returns the smoothed slope.
 * 
 * @param dists The array of distance values.
 * @param len The length of the dists array.
 * @param start A pointer to store the index of the start point.
 * @param end A pointer to store the index of the end point.
 * @param e_slope A pointer to store the error slope.
 * @return The smoothed slope between the start and end points.
 */
float calcEchoSlope(int* dists, int len, int* start, int* end, float* e_slope) {
	// Initialize start and end indices
	*start = 0;
	*end = 0;

	// Find the first valid data point
	for (int i = *start; i < len; i++) {
		if (dists[i] < 5000) {
			*start = i;
			break;
		}
	}

	// Find the last valid data point
	for (int i = *start + 1; i < len; i++) {
		if (dists[i] > 5000) {
			*end = i - 1;
			break;
		}
	}

	// If no valid end point found, set it to the last index
	if (*end == 0) {
		*end = len - 1;
	}

	// Calculate the average slope between start and end points
	float m0_slope = (dists[*end] - dists[*start]) / (*end - *start);

	// Calculate the error slope by comparing each point to the average slope
	*e_slope = 0;
	for (int i = *start + 1; i <= *end; i++) {
		*e_slope += fabs(m0_slope - (dists[i] - dists[i - 1]));
	}
	*e_slope = *e_slope / (*end - *start);

	// Calculate the smoothed slope by averaging slopes between start and end points
	float m1_slope = 0;
	for (int i = 0; i < (*end - *start) / 2; i++) {
		m1_slope += (dists[*end - i] - dists[*start + i]) / (*end - *start - i * 2);
	}
	m1_slope = m1_slope / ((*end - *start) / 2);

	return m1_slope;
}