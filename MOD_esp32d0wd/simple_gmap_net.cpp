#include "simple_gmap.h"
#include "ArduinoJson.h"


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

// create/update/publish
int simple_gmap_publishObj(int id, const char* serialized_obj) {
    
}

// read
int simple_gmap_readObj(int id, char* buffer);

// delete
int simple_gmap_deleteObj(int id);


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

