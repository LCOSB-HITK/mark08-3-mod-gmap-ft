/***
	author: Lucius Pertis
	Complete project details at https://github.com/LCOSB-HITK/
***/

#ifndef LCOSB_ECHO_H
#define LCOSB_ECHO_H

#include "lcosb_config_dev.h"
#include <Arduino.h>
#include "esp_err.h"

#include "lcosb_dataobjs.h"
#include "lcosb_lame.h"

/** lcosb_echo_t
 * 
*/
typedef struct {
	unsigned long ctime;
	uint left;
	uint right;
} lcosb_echo_t;


/** lcosb_echo_bundle_t
 * 
*/
typedef struct {
	//int	gtime; // avg
	int s_gtime;
	int e_gtime;

	int l[15];
	int r[15];
	
	uint8_t size;

	lcosb_unit_kinetic_t unit_pos;
} lcosb_echo_bundle_t;

typedef struct echo_record {
	lcosb_echo_bundle_t echo_bundle;

	unsigned long latest_gvel_upd_at_create;
	int gtime_e;

	// testing
	struct echo_record* next;
} echo_record_t;

typedef struct {
  echo_record_t* LW;  // last write
  echo_record_t* NR;  // next read


  int _dropped;
  int _size;
  int _leak;

} echo_rec_buff_t;

/** lcosb_pl_t
 * 
*/
typedef struct {
	int com[3];

	int	gtime; // avg
	int glen;

	//lcosb_obj_base_t base; // if mapfrag changes, base will contain the com_err
	int acc_err[3]; // plus/minus
} lcosb_echo_pl_t;

// array buff will store lcosb_echo_pl_t directly
// typedef struct {
// 	lcosb_echo_pl_t pl;
// 	// gtime in pl.gtime
// 	// array buff for pl unlike echo
// 	//pl_record_t* next;
// } pl_record_t;

/** ECHO_PL_STORE / pl_rec_buff_t
 * 
 * - malloc() and free() of lcosb_echo_pl_t are to handled by the user
 * 
*/
typedef struct {
	uint8_t LW;  // last write
	uint8_t NR;  // next read


	lcosb_echo_pl_t** BUFF;
	
	int _size;
	int _dropped;
	int _leak;

} pl_rec_buff_t;


//pinout
#define ECHO_PIN_0        15
#define ECHO_TRIGGER      2
#define ECHO_PIN_1        4

// timeout for pulseIn func call; 500cm/0.034cmpus*2 == 29412 ~ 30 ms
#define ECHO_TIMEOUT_MAX 30000
#define ECHO_BUFFER_SIZE_AVG 20


extern echo_rec_buff_t* ECHO_BUFFER;


void SetupEcho();
int initEchoBuff();
void printEchoBuffState();

int initPLBuff(int buff_size);
int echo_writePLStore(lcosb_echo_pl_t* new_pl);
int echo_readPLStore(lcosb_echo_pl_t** data);

echo_record_t* echo_create_rec(unsigned long stime);
int echoBuffWriteObj(echo_record_t* rec);
int echoBuffRead(lcosb_echo_bundle_t* data);
void destroy(echo_rec_buff_t* eb);
void echoBuffClean();
void recordEcho(lcosb_echo_t* copy);

/** getPLBundle
 * 
 * takes in-mem echo data and stores in bundle
 * format/protocol; can be directly shipped to 
 * network (maybe needed to be broken if size>100)
*/
lcosb_pl_bundle_t* lcosb_echo_getPLBundle(int hard_gtime_limit);

#endif // LCOSB_ECHO_H
