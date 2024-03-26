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
    for (int i=1; i<GMAP_MF_PER_GMAP_SIZE; i++)	gmap.fragid[i] = 0;

    gmap.earliest_upd_time = LCOSB_MESH.getNodeTime();
	gmap.latest_upd_time = sgmap_GMAP.earliest_upd_time;

    // do this in setup randomSeed(analogRead(A0));
	gmap.gmap_id = random(1, 1000000);

    return 0;
}

int simple_gmap_createMapFrag(int gmap_id, simple_gmap_mapfrag_t &mapfrag) {
    
    mapfrag.table_count = 0;
	for (int i=0; i<GMAP_TABLES_PER_MF_SIZE; i++)	mapfrag[0].tables[0][0] = 0;

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
        for(int i<0; i<GMAP_MF_PER_GMAP_SIZE; i++)
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
    int com_disp[GMAP_TABLES_PER_MF_SIZE][3];
    for (int i=0, tc=0; i<GMAP_TABLES_PER_MF_SIZE && tc<; i++) {

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

// simple pl_b 2 mapfrag mapping functions
int simple_gmap_mapUaPLB2MapFrag(int plb_id, int frag_id) {
    // (plb_id == -1) => ua_plb
    // (plb_id ==  0) => empty plb
    // (plb_id >   0) => assigned plb
    
    // work for matching plb_id
    for (int i=0; i<GMAP_LOCAL_UAPLB_SIZE; i++) 
    if (sgmap_ua_PL_B[i].plb_id == plb_id && sgmap_ua_PL_B[i].pl_count > 0) { // required ua_plb
        
        if (frag_id == -1) { // chk ALL (local) frags (bounds); assign 2 first match
            for (int j=0; j<GMAP_LOCAL_MF_SIZE; j++) {
                if (sgmap_MAPFRAG[j].frag_id > 0) { // valid frag_id

                    // if the ua_plb is within the bounds of the frag
                    uint8_t member_marker[8] = {0};
                    for (int k=0; k<8; k++) {
                        if (sgmap_MAPFRAG[j].tables[k].obj_id > 0) {
                            if (sgmap_MAPFRAG[j].tables[k].obj_id == sgmap_ua_PL_B[i].pl_obj_id) {
                                member_marker[k] = 1;
                            }
                        }
                    }

                    frag_id = sgmap_GMAP.frags[j];
                    break;
                }
            }
        }
        else { // chk ONLY requested frag_id
            for (int j=0; j<GMAP_LOCAL_MF_SIZE; j++) {
                if (sgmap_MAPFRAG[j].frag_id == frag_id) { // valid local frag_id

                    // if the ua_plb is within the bounds of the frag
                    uint8_t member_marker[GMAP_PLS_PER_PLB_SIZE] = {0};
                    for (int k=0; k<sgmap_ua_PL_B[i].pl_count; k++) {
                        lcosb_echo_pl_t* pl = sgmap_ua_PL_B[i].pls[k];

                        if (pl == NULL) continue; // TODO: chk if pl_count should be the search limit; YES IT SHOULD
                        
                        member_marker[k] = simple_gmap_compare_withinBounds(sgmap_MAPFRAG[j].bounds, pl->com);
                    }

                    simple_gmap_assignUaPLB2MapFrag(i, member_marker, frag_id);

                    break; // we assume no two frags have the same frag_id locally
                }
            }

        }
    }
    
}

int simple_gmap_assignUaPLB2MapFrag(int local_plb_idx, int* member_marker, int assigned_frag_id, int assigned_plb_id) {
    simple_gmap_plb_t* assigned_plb;

    if (assigned_frag_id == -1) { // find new plb
        int new_plb_idx;
        for (new_plb_idx=0; new_plb_idx<GMAP_LOCAL_UAPLB_SIZE; new_plb_idx++)
            if (sgmap_ua_PL_B[new_plb_idx].plb_id == 0) break;
        
        if (new_plb_idx == GMAP_LOCAL_UAPLB_SIZE) 
            return -1; // no free ua_plb
        else
            assigned_plb = &sgmap_ua_PL_B[new_plb_idx]; // fresh uninitialised ua_plb


        // initialise base values
        assigned_plb->plb_id = random(1, 1000000);
        assigned_plb->pl_count = 0;
        assigned_plb->reff_obj_id = assigned_frag_id;
    }
    else { // store in assigned_plb_id
        for (int i=0; i<GMAP_LOCAL_UAPLB_SIZE; i++)
            if (sgmap_ua_PL_B[i].reff_obj_id == assigned_frag_id) {
                assigned_plb = &sgmap_ua_PL_B[i];
                break;
            }
    }

    // copy pls
    int plc=assigned_plb->pl_count;
    for (int i=0; i<GMAP_PLS_PER_PLB_SIZE; i++) if (member_marker[i] == 1) {
        assigned_plb->pls[plc] = (lcosb_echo_pl_t*) malloc(sizeof(lcosb_echo_pl_t));
        if (assigned_plb->pls[plc] == NULL) continue; // malloc failed

        memcpy(assigned_plb->pls[plc], sgmap_ua_PL_B[local_plb_idx].pls[i], sizeof(lcosb_echo_pl_t));

        member_marker[i] = 0; // mark as copied
        assigned_plb->pl_count++;
        plc++;
    }

    for(; plc<GMAP_PLS_PER_PLB_SIZE; plc++) {
        assigned_plb->pls[plc] = NULL;
    }
    
}

// simple recompose mapfrag functions
int simple_gmap_recomposeMapFrag(int frag_id);
int simple_gmap_recomposeObjTable(int frag_id, int obj_id);