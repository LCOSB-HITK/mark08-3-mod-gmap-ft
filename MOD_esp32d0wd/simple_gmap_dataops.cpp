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
        
        if (frag_id == -1) { // we treat this as general distribution of ua_plb to all frags
            uint8_t master_member_mask[GMAP_PLS_PER_PLB_SIZE] = {0};

            // chk ALL (local) frags (bounds)
            for (int j=0; j<GMAP_LOCAL_MF_SIZE; j++) if (sgmap_MAPFRAG[j].frag_id > 0) { // valid frag_id

                // if the ua_plb is within the bounds of the frag
                uint8_t member_mask[GMAP_PLS_PER_PLB_SIZE] = {0}; uint8_t mem_flag = 0;
                for (int k=0, plc=0; k<GMAP_PLS_PER_PLB_SIZE && plc<sgmap_ua_PL_B[i].pl_count; k++) {
                    lcosb_echo_pl_t* pl = sgmap_ua_PL_B[i].pls[k];

                    if (pl == NULL) continue; // TODO: chk if pl_count should be the search limit; YES IT SHOULD
                    
                    member_mask[k] = simple_gmap_compare_withinBounds(sgmap_MAPFRAG[j].bounds, pl->com);
                    master_member_mask[k] = master_member_mask[k] || member_mask[k];

                    if (mem_flag == 0 && member_mask[k] == 1) mem_flag = 1;

                    plc++;
                }
                
                if (mem_flag == 1) { // assign to frag
                    assignUaPLB2MapFrag(i, member_mask, sgmap_MAPFRAG[j].frag_id, 0);
                }

            }
            
            // no frag is found for remaining ua_pls (!master_member_mask)
            int ua_nl=0;
            for (int k=0; k<GMAP_PLS_PER_PLB_SIZE; k++) {
                master_member_mask[k] = !master_member_mask[k]; // invert mask
                if (master_member_mask[k] == 1) ua_nl++;
            }

            if (ua_nl)
                assignUaPLB2MapFrag(i, master_member_mask, -2, 0);

            // after general distribution, destroy current ua_plb
            sgmap_ua_PL_B[i].plb_id = 0;
            sgmap_ua_PL_B[i].pl_count = 0;
            sgmap_ua_PL_B[i].reff_obj_id = 0;
            for (int k=0; k<GMAP_PLS_PER_PLB_SIZE; k++) {
                if (sgmap_ua_PL_B[i].pls[k] != NULL) {
                    free(sgmap_ua_PL_B[i].pls[k]);
                    sgmap_ua_PL_B[i].pls[k] = NULL;
                }
            }
        }
        else { // chk ONLY requested frag_id
            for (int j=0; j<GMAP_LOCAL_MF_SIZE; j++) {
                if (sgmap_MAPFRAG[j].frag_id == frag_id) { // valid local frag_id

                    // if the ua_plb is within the bounds of the frag
                    uint8_t member_mask[GMAP_PLS_PER_PLB_SIZE] = {0};
                    for (int k=0; k<sgmap_ua_PL_B[i].pl_count; k++) {
                        lcosb_echo_pl_t* pl = sgmap_ua_PL_B[i].pls[k];

                        if (pl == NULL) continue; // TODO: chk if pl_count should be the search limit; YES IT SHOULD
                        
                        member_mask[k] = simple_gmap_compare_withinBounds(sgmap_MAPFRAG[j].bounds, pl->com);
                    }

                    assignUaPLB2MapFrag(i, member_mask, frag_id);

                    break; // we assume no two frags have the same frag_id locally
                }
            }

        }
    }
    
}

int assignUaPLB2MapFrag(int local_plb_idx, uint8_t* member_mask, int assigned_frag_id, int assigned_plb_id) {
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
        int i;
        for (i=0; i<GMAP_LOCAL_UAPLB_SIZE; i++)
            if (sgmap_ua_PL_B[i].reff_obj_id == assigned_frag_id && sgmap_ua_PL_B[i].pl_count < GMAP_PLS_PER_PLB_SIZE) {
                assigned_plb = &sgmap_ua_PL_B[i];
                break;
            }
        
        if (i == GMAP_LOCAL_UAPLB_SIZE)
            return assignUaPLB2MapFrag(local_plb_idx, member_mask, assigned_frag_id, -1);
    }

    // copy pls
    int plc=assigned_plb->pl_count;
    int mark_idx;
    for (mark_idx=0; mark_idx<GMAP_PLS_PER_PLB_SIZE && plc<GMAP_PLS_PER_PLB_SIZE; mark_idx++) if (member_mask[mark_idx] == 1) {
        assigned_plb->pls[plc] = (lcosb_echo_pl_t*) malloc(sizeof(lcosb_echo_pl_t));
        if (assigned_plb->pls[plc] == NULL) continue; // malloc failed

        memcpy(assigned_plb->pls[plc], sgmap_ua_PL_B[local_plb_idx].pls[mark_idx], sizeof(lcosb_echo_pl_t));

        member_mask[mark_idx] = 0; // mark as copied
        assigned_plb->pl_count++;
        plc++;
    }

    // assumed ?
    //for(; plc<GMAP_PLS_PER_PLB_SIZE; plc++) assigned_plb->pls[plc] = NULL;

    if (mark_idx == GMAP_PLS_PER_PLB_SIZE) 
        return 0; // all copied
    else
        assignUaPLB2MapFrag(local_plb_idx, member_mask, assigned_frag_id, assigned_plb_id);

}

/** simple recompose mapfrag functions
 * 
 * it is considered that the mapfrag is already updated with its base attributes
 * and more importantly, the tables/objs, ids (inclusion of an obj) are updated
*/ 
int simple_gmap_recomposeMapFrag(int frag_id) {
    // find the frag_id
    int frag_idx;
    for (frag_idx=0; frag_idx<GMAP_LOCAL_MF_SIZE; frag_idx++)
        if (sgmap_MAPFRAG[frag_idx].frag_id == frag_id) break;
    
    if (frag_idx == GMAP_LOCAL_MF_SIZE) return -1; // frag_id not found

    simple_gmap_mapfrag_t *mf = &sgmap_MAPFRAG[frag_idx];

    // delagated to refresh mapfrag
    // update base attributes (bounds)
    // delagated

    // ONLY recompose for the tables/objs in the frag
    // will be handled here
    
    // find all the ua_plbs assigned to this frag
    int ua_plb_idx[GMAP_LOCAL_UAPLB_SIZE] = {0};
    int ua_plb_count = 0;
    for (int i=0; i<GMAP_LOCAL_UAPLB_SIZE; i++)
        if (sgmap_ua_PL_B[i].reff_obj_id == frag_id) {
            ua_plb_idx[ua_plb_count] = i;
            ua_plb_count++;
        }

    if (ua_plb_count == 0) return -2; // no ua_plb assigned to this frag


    // create segregated plbs for each obj_table (updated)
    for (int i=0, tc=0; i<GMAP_TABLES_PER_MF_SIZE && tc<mf->table_count; i++) {
        if (mf->tables[i][0] == 0) continue; // invalid obj_id

        // check all uaplbs if they can be assigned to this obj_table
        // THIS MAYBE HIGHLT UNOPTIMISED
        for (int j=0; j<ua_plb_count; j++) {
            simple_gmap_plb_t *plb = &sgmap_ua_PL_B[ua_plb_idx[j]];

            // check if plb has the obj_table
            for (int k=0; k<plb->pl_count; k++) {
                lcosb_echo_pl_t *pl = plb->pls[k];
                if (pl == NULL) continue;

                if (pl->acc_err[0] == mf->tables[i][1] && pl->acc_err[1] == mf->tables[i][2] && pl->acc_err[2] == mf->tables[i][3]) {
                    // assign to the obj_table
                    simple_gmap_assignPL2ObjTable(pl, mf->tables[i][0]);
                    break;
                }
            }
        }

    }
}

int simple_gmap_recomposeObjTable(int obj_id, int frag_id) {
    // get the obj_table
    int obj_idx;
    for (obj_idx=0; obj_idx<GMAP_LOCAL_OBJ_TABLE_SIZE; obj_idx++)
        if (sgmap_OBJ_TABLE[obj_idx].obj_id == obj_id) break;
    if (obj_idx == GMAP_LOCAL_OBJ_TABLE_SIZE) return -1; // obj_id not found

    simple_gmap_obj_table_t* obj_table = &sgmap_OBJ_TABLE[obj_idx];
    simple_gmap_recalcObjTable(obj_table, 1);

    // test basic in-bounds
    int bounds[2][2][2] = { // highly depended on llm storing order; considering quadrant wise
        {{obj_table->llm[0][0], obj_table->llm[0][1]}, {obj_table->llm[0][0], obj_table->llm[1][1]}}
    };

    // get hints from sgmap_obj_plb
    for (int i=0; i<GMAP_LOCAL_OBJPLB_SIZE; i++) if (sgmap_obj_PL_B[i].reff_obj_id == obj_id) {

        for (int j=0; j<sgmap_obj_PL_B[i].pl_count; j++) {
            lcosb_echo_pl_t* pl = sgmap_obj_PL_B[i].pls[j];
            if (pl == NULL) continue;

            
        }
    }
}

void recomposeObjTable(lcosb_echo_pl_t* pl, simple_gmap_obj_table_t* obj_table) {
    // chk basic in-bounds
    simple_gmap_recalcObjTable(obj_table, 1);
    int bounds[2][2];

    bounds = {{obj_table->llm[0][0], obj_table->llm[0][1]}, {obj_table->llm[1][0], obj_table->llm[1][1]}};

    // update com
    for (int i=0; i<3; i++) {
        obj_table->com[i] = (obj_table->com[i] * obj_table->gtime + pl->com[i]) / (obj_table->gtime + 1);
    }

    // update gtime and acc
    obj_table->gtime++;
    obj_table->acc = (obj_table->acc + pl->acc_err[0] + pl->acc_err[1] + pl->acc_err[2]) / 3;
}