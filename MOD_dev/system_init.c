#include "lcosb_gmap.h"
#include "lcosb_lame.h"

void onBoot() {
    INIT_GMAP();
}

void mark08_3_sysInit() {
    // lame/gmap: units with valid (shared) gpos and gmap repo_id


    // gmap/repo: get gmap into memory
    if (lcosb_repo_chkInMem(GMAP_GMAP.base.repo_id) == 0) {
        // if qourum 
        // - ask quorum for existing gmap

        // - if not poll 

        // if no quorum create gmap
        lcosb_gmap_createL5Gmap();
    }

    if (lcosb_repo_chkInMem(GMAP_GMAP.base.repo_id) == 0)   return 1;

    // gmap: load mapfrag
    if (GMAP_CURR_MAPFRAG[GMAP_CURR_MAPFRAG_IDX] == NULL) {
        // load mapfrag from repo
        lcosb_gmap_env_gpos2NodeAddr(GMAP_GMAP.base.repo_id, getGPos());

        // repo: search gmap entry in L5_obj_reg (available)
        // gmap: search frag_tree (null)
        // gmap: recursively create frag_tree_node
        // gmap: create mapfrag (empty)
        // gmap/repo: send mapfrag

    }

    if (GMAP_CURR_MAPFRAG[GMAP_CURR_MAPFRAG_IDX] == NULL) return 2;

    // echo: record echo
    // echo: create echo_bundle
    // echo: pack into pl
    // echo/repo: publish pl -> mapfrag

    
    // repo: create/store-into unassigned ua_pl_b_reg
    // gmap: compatify (pl -> pl_b) (pl_b -> pl_b)
    // repo: from ua_pl_b_reg foward pl_b -> gmap
    // gmap/repo: distribute/hint mapfrags <- pl_b(s)
    // gmap: upd/ref objs
    // gmap: frag_tree refresh
    // gmap::map_frag: pending/distributed pl_b(s)
    // gmap::map_frag: chk obj owernership/hint to all objs (empty)
    // gmap::map_frag: chk obj_table inst.
    // gmap::map_frag: hint the obj_table and schedule recompose()
    // gmap::map_frag: (if no obj owenership found) creation routine
    // gmap::map_frag: create obj
    // gmap::map_frag: create obj_table
    // gmap::map_frag: chk merging
    // gmap::map_frag: upd gmap
    // repo: gc ua_pl_b_reg
}