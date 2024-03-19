/***
	author: Lucius Pertis
	Complete project details at https://github.com/LCOSB-HITK/
***/


#include "simple_gmap.h"

#include "task_basic.h"

/** GMap initiliation Routine
 * 
*/
void initGMap() {
	// initialize the gmap and mapfrag

	// create the unregisted pl_b store

	// publish the gmap to the network

	// schedule routines to check for updates/consistancy checks

	// schedule routines to recompose the mapfrag

	// schedule routines to compose local pls and publish them




	task_registerRoutine(PROCESS_GMAP, ROUTINE_GMAP_ECHO_CAPTURE, &startEchoRecordTimed);
	task_registerRoutine(PROCESS_GMAP, ROUTINE_GMAP_FLUSH_ECHO_2_PL, &convertEcho2PL);
}

/** GMap maintainer Routine
 * 
*/
void maintainGMap() {
	// register the routine
	task_registerRoutine(PROCESS_GMAP, ROUTINE_GMAP_PLB_PUBLISH, &echo_writePLStore);
	task_registerRoutine(PROCESS_GMAP, ROUTINE_GMAP_MAPFRAG_RECOMPOSE, &echo_recomposeMapFrag);
	task_registerRoutine(PROCESS_GMAP, ROUTINE_GMAP_L5_UPDATE, &echo_updateL5);
}

void simple_gmap_recalcObjTable(simple_gmap_obj_table_t* obj_table, int COM2LLM) {
	if (COM2LLM) { // calc the obj_table llm from com

	} else { // calc the obj_table com from llm
		obj_table->com[0] = 0;		
		for(int i = 0; i < 4; i++)
			obj_table->com[0] += obj_table->llm[i][0];
		obj_table->com[0] /= 4;
		
		obj_table->com[1] = 0;
		for(int i = 0; i < 4; i++)
			obj_table->com[1] += obj_table->llm[i][1];
		obj_table->com[1] /= 4;

		// avereage of the slope of lines llm[0] to llm[2] and llm[1] to llm[3]
		const float slope_02 = (obj_table->llm[0][1] - obj_table->llm[2][1]) / (obj_table->llm[0][0] - obj_table->llm[2][0]);
		const float slope_13 = (obj_table->llm[1][1] - obj_table->llm[3][1]) / (obj_table->llm[1][0] - obj_table->llm[3][0]);
		obj_table->com[2] = (slope_02 + slope_13)*1000 / 2; // in mrad
	}
}