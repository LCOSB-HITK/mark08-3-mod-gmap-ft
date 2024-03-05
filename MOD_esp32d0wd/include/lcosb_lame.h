/***
	author: Lucius Pertis
	Complete project details at https://github.com/LCOSB-HITK/
***/

#ifndef LCOSB_LAME_H
#define LCOSB_LAME_H

#include "lcosb_config_dev.h"
#include "Arduino.h"
#include "esp_err.h"

void getGPos(int* );
void getGVel(int* );

void setGPos(int[3]);
void setGVel(double[3]);

void reCalcTraj();
void updateGPos();

void _invCalcTraj(int* calc_mspeed, double rvel_avg, double rvel_omega, double rvel_rcurve);
void lcosb_lame_moveGPos(int* gvel, int t, lcosb_gops_t* initpos, lcosb_gpos_t* finalpos); // t:time in millisec

#endif // LCOSB_LAME_H
