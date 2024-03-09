/***
	author: Lucius Pertis
	Complete project details at https://github.com/LCOSB-HITK/
***/

#ifndef LCOSB_LAME_H
#define LCOSB_LAME_H

#include "lcosb_config_dev.h"
#include <Arduino.h>
#include "esp_err.h"

// for lcosb_gpos_t
#include "lcosb_dataobjs.h"

void getGPos(int* );
void getGVel(int* );

void setGPos(int[3]);
void setGVel(double[3]);

void _reCalcTraj();
void reCalcTraj();

void updateGPos();

void _invCalcTraj(int* calc_mspeed, double rvel_avg, double rvel_omega, double rvel_rcurve);
void lcosb_lame_moveGPos(int* gvel, int t, lcosb_gpos_t* initpos, lcosb_gpos_t* finalpos); // t:time in millisec

unsigned long lcosb_lame_getLastGvelUpdate();
void getGPosErr(int* );

#endif // LCOSB_LAME_H
