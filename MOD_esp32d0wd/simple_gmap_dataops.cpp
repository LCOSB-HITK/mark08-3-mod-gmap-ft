#include "simple_gmap.h"
#include "lcosb_lame.h"

// simple create funtions
int simple_gmap_createGMap(simple_gmap_t &gmap) {

    int gpos[3]; getGPos(gpos);
    gmap.bounds[0][0] = gpos[0];
	gmap.bounds[0][1] = gpos[1];
	gmap.bounds[1][0] = gpos[0];
	gmap.bounds[1][1] = gpos[1];

    gmap.frag_count = 0;
    for (int i=1; i<16; i++)	gmap.fragid[i] = 0;

    gmap.earliest_upd_time = LCOSB_MESH.getNodeTime();
	gmap.latest_upd_time = sgmap_GMAP.earliest_upd_time;

    // do this in setup randomSeed(analogRead(A0));
	gmap.gmap_id = random(1, 1000000);

    return 0;
}

int simple_gmap_createMapFrag(int gmap_id, simple_gmap_mapfrag_t &mapfrag) {
    
    mapfrag.table_count = 0;
	for (int i=0; i<8; i++)	mapfrag[0].tables[0][0] = 0;

    mapfrag.bounds[0][0] = 0;
    mapfrag.bounds[0][1] = 0;
    mapfrag.bounds[1][0] = 0;
    mapfrag.bounds[1][1] = 0;

    mapfrag.plb_store_tag = 0;

    int ct;
    if (sgmap_GMAP.gmap_id == gmap_id)  ct = sgmap_GMAP.latest_upd_time;
    else                                ct = LCOSB_MESH.getNodeTime();

    mapfrag.earliest_upd_time   = ct;
    mapfrag.latest_upd_time     = ct;

    // do this in setup randomSeed(analogRead(A0));
    // incorporate gmap_id into the frag_id somehow
    //mapfrag.gmap_id = gmap_id;
    mapfrag.frag_id = random(1, 1000000);
    if (sgmap_GMAP.gmap_id == gmap_id) {
        for(int i<0; i<16; i++)
            if(!(sgmap_GMAP.frags[i] > 0)) { // invalid frag_id
                sgmap_GMAP.frags[i] = mapfrag.frag_id;
                sgmap_GMAP.frag_count++;
                break;
            }
    }

    return 0;
}

int simple_gmap_createTable(int frag_id, simple_gmap_obj_table_t &table) {
    table.com[0] = 0;
    table.com[1] = 0;
    table.com[2] = 0;

    table.gtime = -1;
    table.acc = 0;

    table.obj_id = random(1, 1000000);

    return 0;
}


// simple equating and merging functions
int simple_gmap_eqGMap(simple_gmap_t *gmap1, simple_gmap_t *gmap2);
int simple_gmap_eqMapFrag(simple_gmap_mapfrag_t *mapfrag1, simple_gmap_mapfrag_t *mapfrag2) {
    // if all the tables/objs are same or have the same relative displacement (and rotation)
    int com_disp[8][3];
    for (int i=0, tc=0; i<8 && tc<; i++) {

        if (simple_gmap_eqTable(mapfrag1.tables[i], mapfrag2.tables[i]) == 0) {
            return 0;
        }
    }
}
int simple_gmap_mergeMapFrag(simple_gmap_mapfrag_t *mapfrag1, simple_gmap_mapfrag_t *mapfrag2);

int simple_gmap_eqTable(int *table1, int *table2, float threshold_factor = 1, int *com_disp = NULL) {
    // if com is same within a threshold
    int disp[3] = {0};

    // calc displacement
    for (int i=0; i<3; i++) {
        disp[i] = table1.com[i] - table2.com[i];
    }
    // copy to com_disp
    if (com_disp != NULL)
        for (int i=0; i<3; i++)
            com_disp[i] = disp[i];
    

    // if com within a threshold
    if (    disp[0] < GMAP_TRANS_ERR_THRESHOLD * threshold_factor && 
            disp[1] < GMAP_TRANS_ERR_THRESHOLD * threshold_factor && 
            disp[2] < GMAP_ROT_ERR_THRESHOLD   * threshold_factor)
        return 0;
    else
        return 1;
}

// merge into table1
// it is assumed that the tables are equal or needs to be merged anyhow
int simple_gmap_mergeTable(simple_gmap_obj_table_t *table1, simple_gmap_obj_table_t *table2) {
    // change com to the average of the two coms
    // weightage to the one with the higher gtime and acc
    float w1 = table1->gtime * table1->acc;
    float w2 = table2->gtime * table2->acc;

    for (int i=0; i<3; i++) {
        table1->com[i] = (table1->com[i] * w1 + table2->com[i] * w2) / (w1 + w2);
    }

    // update gtime and acc
    table1->gtime = (table1->gtime * table1->acc + table2->gtime * table2->acc) / (table1->acc + table2->acc);
    table1->acc = (table1->acc + table2->acc) / 2;

    return 0;
}

