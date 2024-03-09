/***
	author: Lucius Pertis
	Complete project details at https://github.com/LCOSB-HITK/
***/

#ifndef LCOSB_MOTOR_H
#define LCOSB_MOTOR_H

#include "lcosb_config_dev.h"
#include "Arduino.h"
#include "esp_err.h"


// PWM analog state for motors
#define PWM_CHANNEL_L     1
#define PWM_CHANNEL_R     2

// use 12 bit precission for LEDC timer
#define LEDC_TIMER_12_BIT  12
#define LEDC_DUTY_MAX_FLOAT_12_BIT 4095.0
#define PWM_BASE_FREQ     10000

#define MOTOR_SPEED_MAX   10

// mechtronic specific empirical behaviour 
#define MOTOR_DUTY_MIN_12_BIT 1966 // 48% of 4096


extern int motorspeed[2];   //speed of motor in range [-10, +10]

void SetupMotorControls();

// custom for motors
void pwmWriteMotors(uint8_t channel, float value, float valueMax);

void updateMotorOutput();

void motorKill();

int getMotorSpeed(uint8_t right);

void incMotorSpeed(uint8_t right, int8_t inc);

// avoid using this func
void setMotorSpeed(int left_speed, int right_speed);

// very general and implementation are varied
void steerRover(int dist, int steer);
void inertDecayPower();
void inertDecaySteer();

// lame assisted
int moveRoverByDist(int dist, int tp); // milli meters; msec
int	rotateRoverByAngle(int theta, int tp); // milli radian; msec


#endif // LCOSB_MOTOR_H
