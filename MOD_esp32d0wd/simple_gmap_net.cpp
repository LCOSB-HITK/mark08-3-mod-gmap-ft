#include "simple_gmap.h"
#include "ArduinoJson.h"

#include "lcosb_net.h"
#include "lcosb_mesh_dataops.h"

// find a better way to do this or a separate library for this
#include <iostream>
#include <sstream>
#include <vector>

void extractIntegers(const char* str, std::vector<int>& integers) {
    std::istringstream iss(str);
    int num;
    while (iss >> num) {
        integers.push_back(num);
    }
}

// send obj req
int simple_gmap_sendObjReq(OBJ_REQ_TYPE req_type, const char* data, const char * extra_data) {
    char buffer[200];
    int cw = snprintf(buffer, 200,
                "{ \"type\": \"gmap_mdo\", \"method\": %d, \"data\": \"%s\", \"extra_data\": \"%s\" }",
                req_type, String(data), String(extra_data);

    if (cw < 0) return -1;

    LCOSB_MESH.sendBroadcast(buffer);
    return 0;
}

// create/update/publish
int simple_gmap_publishObj(int obj_id, OBJ_TYPE obj_type, OBJ_REQ_TYPE req_type, const char* net_msg) {
    char buffer[100];

    // create obj str_digest
    if (obj_type == TYPE_GMAP) {
        // chk if obj_id is valid
        if (obj_id != sgmap_GMAP.gmap_id) return -1;

        int ret = simple_gmap_serializeGMap(&sgmap_GMAP, buffer);
        if (!ret)
            ret = simple_gmap_sendObjReq(req_type, buffer, net_msg);
        
        return ret;
    } 
    else if (obj_type == TYPE_MAPFRAG) {
        // chk if obj_id is valid
        int i;
        for (i=0; i<16; i++) {
            if (sgmap_GMAP.frags[i] == obj_id) break;
        }   if (i == 15) return -1;

        int ret = simple_gmap_serializeMapFrag(&sgmap_MAP_FRAGS[i], buffer);
        if (!ret)
            ret = simple_gmap_sendObjReq(req_type, buffer, net_msg);
        
        return ret;
    }
    else if (obj_type == TYPE_OBJ_TABLE) {
        // chk if obj_id is valid
        int i;
        for (i=0; i<16; i++) {
            if (sgmap_OBJ_TABLE[i].obj_id == obj_id) break;
        }   if (i == 15) return -1;

        int ret = simple_gmap_serializeObjTable(&sgmap_OBJ_TABLE[i], buffer);
        if (!ret)
            ret = simple_gmap_sendObjReq(req_type, buffer, net_msg);
        
        return ret;
    }
    else if (obj_type == TYPE_PLB) {
        // chk if obj_id is valid
        int i;
        for (i=0; i<16; i++) {
            if (sgmap_ua_PL_B[i].plb_id == obj_id) break;
        }   if (i == 15) return -1;

        // may require more buffer size
        int ret = simple_gmap_serializePLB(&sgmap_ua_PL_B[i], buffer);
        if (!ret)
            ret = simple_gmap_sendObjReq(req_type, buffer, net_msg);

        return ret;
    }

    return -2;
}

// read or get
int simple_gmap_readObj(int id, OBJ_TYPE obj_type, const char* net_msg) {
    char buffer[100];
    int cw = snprintf(buffer, 100,
                    "{ \"obj_id\": %d, \"obj_type\": %d }",
                    id, obj_type);

    if (cw < 0) return -1;
    return simple_gmap_sendObjReq(mdo_READ, String(buffer), net_msg);
}

// delete or invalidate
int simple_gmap_deleteObj(int id, OBJ_TYPE obj_type, const char* net_msg) {
    char buffer[100];
    int cw = snprintf(buffer, 100,
                    "{ \"obj_id\": %d, \"obj_type\": %d }",
                    id, obj_type);

    if (cw < 0) return -1;
    return simple_gmap_sendObjReq(mdo_DELETE, String(buffer), net_msg);
}


// recieve obj req
// const char* needs to be freed
int simple_gmap_recvObjReq(const char* data, const char* extra_data, OBJ_REQ_TYPE req_type) {
    if (data == NULL || extra_data == NULL) return -1;
    int ret = -1, new_obj_idx;

    // log the extra_data
    Serial.print(F("Extra Data recv: "));
    Serial.println(extra_data);

    // get the data
    JsonDocument obj_doc;
    DeserializationError obj_error = deserializeJson(obj_doc, data);
    if (obj_error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(obj_error.c_str());
        return -3;
    }

    int obj_type = obj_doc["obj_type"];
    
    if (req_type == mdo_CREATE || req_type == mdo_READ || mdo_UPDATE) {
        
        if (obj_type == TYPE_GMAP) {
            simple_gmap_t gmap;
            ret = simple_gmap_deserializeGMap(&gmap, data);
            if (!ret) {
                // add to sgmap_GMAP
                ret = simple_gmap_eqGMap(&gmap, NULL);
            }
        }
        else if (obj_type == TYPE_MAPFRAG) {
            // add to sgmap_MAP_FRAGS
            for (new_obj_idx=0; new_obj_idx<5; new_obj_idx++) {
                if (!(sgmap_MAPFRAG[new_obj_idx].frag_id > 0)) break;
            }   if (new_obj_idx == 15) ret = -4; 

            if (!ret) ret =  simple_gmap_deserializeMapFrag(&sgmap_MAPFRAG[new_obj_idx], data);
            if (ret) sgmap_MAPFRAG[new_obj_idx].frag_id = -1;

        }
        else if (obj_type == TYPE_OBJ_TABLE) {
            // add to sgmap_OBJ_TABLE
            for (new_obj_idx=0; new_obj_idx<16; new_obj_idx++) {
                if (!(sgmap_OBJ_TABLE[new_obj_idx].frag_id > 0)) break;
            }   if (new_obj_idx == 15) ret = -4; 

            if (!ret) ret =  simple_gmap_deserializeObjTable(&sgmap_OBJ_TABLE[new_obj_idx], data);
            if (ret) sgmap_OBJ_TABLE[new_obj_idx].frag_id = -1;

        }
        else if (obj_type == TYPE_PLB) {
            // add to sgmap_ua_PL_B
            for (new_obj_idx=0; new_obj_idx<16; new_obj_idx++) {
                if (!(sgmap_ua_PL_B[new_obj_idx].frag_id > 0)) break;
            }   if (new_obj_idx == 15) ret = -4; 

            if (!ret) ret =  simple_gmap_deserializePLB(&sgmap_ua_PL_B[new_obj_idx], data);
            if (ret) sgmap_ua_PL_B[new_obj_idx].frag_id = -1;

        }

        ret = -2;
    }
    else
    if (req_type == mdo_DELETE) {
        if (obj_type == TYPE_MAPFRAG) {
            // delete from sgmap_MAP_FRAGS
            int obj_id = obj_doc["obj_id"];
            ret = simple_gmap_eqMapFrag(NULL, &obj_id);
        }
        else if (obj_type == TYPE_OBJ_TABLE) {
            // delete from sgmap_OBJ_TABLE
            int obj_id = obj_doc["obj_id"];
            ret = simple_gmap_eqObjTable(NULL, &obj_id);
        }
        else if (obj_type == TYPE_PLB) {
            // delete from sgmap_ua_PL_B
            int obj_id = obj_doc["obj_id"];
            ret = simple_gmap_eqPLB(NULL, &obj_id);
        }

        ret = -2;
    }

    // if obj is sent for update, we call higher lvl objs to update wrt the obj
    if (req_type == mdo_UPDATE && ret == 0) {

        //if(obj_type == TYPE_GMAP) // already is updated by eqGMA
        
        if (obj_type == TYPE_MAPFRAG) {
            // update sgmap_GMAP wrt sgmap_MAPFRAG
            ret = simple_gmap_updateGMapFrag(new_obj_idx);
        }
        else
        if (obj_type == TYPE_OBJ_TABLE) {
            // update sgmap_MAPFRAG wrt sgmap_OBJ_TABLE
            ret = simple_gmap_updateMapFragTable(new_obj_idx);
        }

        ret = -2;
    }

    free(data);
    free(extra_data);
    return ret;
}

// net gmap objects serialization and deserialization
int simple_gmap_serializeGMap(simple_gmap_t *gmap, char *buffer) {
    String frags_str = String(gmap->frag_count);

    for(int i=0, fc=0; i<16 && fc<gmap->frag_count; i++) 
        if(gmap->frags[i] > 0) {
            frags_str.concat(" " + String(gmap->frags[i]));
            fc++;
        }

    // serialize gmap object
    int cw = snprintf(buffer, 100,
                    "{ \"obj_id\": %d, \"obj_type\": %d, \"str_digest\": \"%d %d %d %d %d %d %d %d\", \"frags\": \"%s\" }",
                    gmap->gmap_id, TYPE_GMAP,
                    gmap->gmap_id,
                    gmap->earlist_upd_time, gmap->latest_upd_time,
                    gmap->bounds[0][0], gmap->bounds[0][1],
                    gmap->bounds[1][0], gmap->bounds[1][1],
                    gmap->frag_count,
                    frags_str.c_str());

    // return 0 on success
    if (cw > 0) return 0;
    else        return -1;
}
int simple_gmap_deserializeGMap(simple_gmap_t *gmap, const char *buffer) {
    // deserialize gmap object
    if(gmap == NULL) return -1;

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, buffer);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return -2;
    }

    if ((int doc["obj_type"]) != TYPE_GMAP) return -3;

    // all good, populate gmap object
    const char* str_digest = doc["str_digest"];
    sscanf(str_digest, "%d %d %d %d %d %d %d %d", 
            &gmap->gmap_id,
            &gmap->earlist_upd_time, &gmap->latest_upd_time,
            &gmap->bounds[0][0], &gmap->bounds[0][1],
            &gmap->bounds[1][0], &gmap->bounds[1][1],
            &gmap->frag_count);
    
    

    // populate frags
    const char* frags_str = doc["frags"];
    int fc; sscanf(frags_str, "%d", &fc);

    // bad serialization
    if (gmap->gmap_id != (int doc["obj_id"]) || fc != gmap->frag_count)
        return -4;

    std::vector<int> result;
    extractIntegers(frags_str, result);

    for(int i=0; i<16; i++) {
        if(i < gmap->frag_count) {
            gmap->frags[i] = result[i+1];
        }
        else {
            gmap->frags[i] = -1;
        }
    }

    return 0;
}

int simple_gmap_serializeMapFrag(simple_gmap_mapfrag_t *mapfrag, char *buffer) {
    String tables_str = String(mapfrag->table_count);
    for(int i=0, tc=0; i<16 && tc<mapfrag->table_count; i++)
        if(mapfrag->tables[i][0] > 0) {
            tables_str.concat(
                        " " 
                        + String(mapfrag->tables[i][0]) + " "
                        + String(mapfrag->tables[i][1]) + " "
                        + String(mapfrag->tables[i][2]) + " "
                        + String(mapfrag->tables[i][3])
            );
            tc++;
        }
    
    // serialize mapfrag object
    int cw = snprintf(buffer, 100,
                    "{ \"obj_id\": %d, \"obj_type\": %d, \"str_digest\": \"%d %d %d %d %d %d %d %d\", \"tables\": \"%s\",
                    \"plb_store_tag\": %d }",
                    mapfrag->mapfrag_id, TYPE_MAPFRAG,
                    mapfrag->mapfrag_id,
                    mapfrag->earlist_upd_time, mapfrag->latest_upd_time,
                    mapfrag->bounds[0][0], mapfrag->bounds[0][1],
                    mapfrag->bounds[1][0], mapfrag->bounds[1][1],
                    mapfrag->table_count,
                    tables_str.c_str(),
                    mapfrag->plb_store_tag);
    
    // return 0 on success
    if (cw > 0) return 0;
    else        return -1;
}
int simple_gmap_deserializeMapFrag(simple_gmap_mapfrag_t *mapfrag, char *buffer) {
    // deserialize mapfrag object
    if(mapfrag == NULL) return -1;

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, buffer);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return -2;
    }

    if ((int doc["obj_type"]) != TYPE_MAPFRAG) return -3;

    // all good, populate mapfrag object
    const char* str_digest = doc["str_digest"];
    sscanf(str_digest, "%d %d %d %d %d %d %d %d", 
            &mapfrag->mapfrag_id,
            &mapfrag->earlist_upd_time, &mapfrag->latest_upd_time,
            &mapfrag->bounds[0][0], &mapfrag->bounds[0][1],
            &mapfrag->bounds[1][0], &mapfrag->bounds[1][1],
            &mapfrag->table_count);
    
    // populate tables
    const char* tables_str = doc["tables"];
    int tc; sscanf(tables_str, "%d", &tc);

    // bad serialization
    if (mapfrag->mapfrag_id != (int doc["obj_id"]) || tc != mapfrag->table_count)
        return -4;

    std::vector<int> result;
    extractIntegers(tables_str, result);

    for(int i=0; i<8; i++) {
        if(i < mapfrag->table_count) {
            mapfrag->tables[i][0] = result[i*4+1];
            mapfrag->tables[i][1] = result[i*4+2];
            mapfrag->tables[i][2] = result[i*4+3];
            mapfrag->tables[i][3] = result[i*4+4];
        }
        else {
            mapfrag->tables[i][0] = -1;
        }
    }

    mapfrag->plb_store_tag = doc["plb_store_tag"];

    return 0;
}

int simple_gmap_serializeObjTable(simple_gmap_obj_table_t *obj_table, char *buffer) {
    
    // serialize obj_table
    int cw = snprintf(buffer, 100,
                    "{ \"obj_id\": %d, \"obj_type\": %d, \"llm\": [ %d, %d, %d, %d, %d, %d, %d, %d ], \"gtime\": %d, \"acc\": %d }", // using json array because its a small array
                    obj_table->obj_id, TYPE_OBJ_TABLE,
                    obj_table->llm[0][0], obj_table->llm[0][1],
                    obj_table->llm[1][0], obj_table->llm[1][1],
                    obj_table->llm[2][0], obj_table->llm[2][1],
                    obj_table->llm[3][0], obj_table->llm[3][1],
                    obj_table->gtime, obj_table->acc
                    );
    
    // return 0 on success
    if (cw > 0) return 0;
    else        return -1;

}
int simple_gmap_deserializeObjTable(simple_gmap_obj_table_t *obj_table, char *buffer) {
    // deserialize obj_table
    if(obj_table == NULL) return -1;

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, buffer);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return -2;
    }

    if ((int doc["obj_type"]) != TYPE_OBJ_TABLE) return -3;

    // all good, populate obj_table object
    obj_table->obj_id   = doc["obj_id"];
    obj_table->gtime    = doc["gtime"];
    obj_table->acc      = doc["acc"];

    JsonArray llm_jarray = doc["llm"];
    for(int i=0; i<4; i++) {
        for(int j=0; j<2; j++) {
            obj_table->llm[i][j] = llm_jarray[i*2 + j];
        }
    }

    
    // calc com from llm
    simple_gmap_recalcObjTable(obj_table, 0);

    return 0;
}

int serializePL(lcosb_echo_pl_t *pl, char *buffer) {
    if (pl == NULL) return -1;
    int cw = snprintf(buffer, 100,
                    "{ \"com\": [ %d, %d, %d, ], \"gtime\": %d, \"glen\": %d, \"acc_err\": [ %d, %d, %d ] }",
                    pl->com[0], pl->com[1], pl->com[2],
                    pl->gtime, pl->glen,
                    pl->acc_err[0], pl->acc_err[1], pl->acc_err[2]
                    );
    
    // return 0 on success
    if (cw > 0) return 0;
    else        return -1;
}
int simple_gmap_serializePLB(simple_gmap_plb_t *plb, char *buffer) {

    int cw = snprintf(buffer, 100,
                "{ \"obj_id\": %d, \"obj_type\": %d,  \"reff_obj_id\": %d,  \"pl_count\": %d, \"pls\": [ ",
                plb->plb_id, TYPE_PLB,
                plb->reff_obj_id, plb->pl_count
                );

    if (cw < 0) return -1;

    char pl_buffer[100];
    for(int i=0, plc=0; i<8 && plc<plb->pl_count; i++) {
        if(plb->pls[i] != NULL) {
            cw = serializePL(plb->pls[i], pl_buffer);
            if (cw > 0) {
                strcat(buffer, pl_buffer);
                strcat(buffer, " , ");
            }
            plc++;
        }
    }

    strcat(buffer, " {} ] }");

    if (cw > 0) return 0;
    else        return -1;
}
int simple_gmap_deserializePLB(simple_gmap_plb_t *plb, char *buffer) {
    // deserialize plb
    if(plb == NULL) return -1;

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, buffer);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return -2;
    }

    if ((int doc["obj_type"]) != TYPE_PLB) return -3;

    // all good, populate plb object
    plb->obj_id     = doc["obj_id"];
    plb->reff_obj_id= doc["reff_obj_id"];
    plb->pl_count   = doc["pl_count"];

    // populate pls
    for(int i=0; i<8; i++) { // set default to NULL
        plb->pls[i] = NULL;
    }
    JsonArray pls_jarray = doc["pls"];
    for(int i=0, plbc=0; i<8; i++) {
        if (plbc<pl_count) {
            plb->pls[plbc] = (lcosb_echo_pl_t*) malloc(sizeof(lcosb_echo_pl_t));


            if (plb->pls[plbc] == NULL) {
                const char* pl_str = pls_jarray[i];
                if(pl_str == NULL) continue;

                JsonDocument pl_doc = deserializeJson(pl_str);

                JsonArray com_jarray = pl_doc["com"];
                JsonArray acc_err_jarray = pl_doc["acc_err"];

                plb->pls[plbc]->com[0] = com_jarray[0];
                plb->pls[plbc]->com[1] = com_jarray[1];
                plb->pls[plbc]->com[2] = com_jarray[2];

                plb->pls[plbc]->gtime = pl_doc["gtime"];
                plb->pls[plbc]->glen = pl_doc["glen"];

                plb->pls[plbc]->acc_err[0] = acc_err_jarray[0];
                plb->pls[plbc]->acc_err[1] = acc_err_jarray[1];
                plb->pls[plbc]->acc_err[2] = acc_err_jarray[2];

                free(pl_str);
                plbc++;
            }
        }
    }    

    return 0;
}

