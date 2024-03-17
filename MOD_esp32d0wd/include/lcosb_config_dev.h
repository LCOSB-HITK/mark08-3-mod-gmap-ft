/*********
  author: Lucius Pertis
  Complete project details at  https://github.com/LCOSB-HITK/
*********/

//lcosb_motor.h
#ifndef LCOSB_CONFIG
#define LCOSB_CONFIG

// debug
#define LCOSB_DEBUG_LVL 4
#define LCOSB_ERR 2
#define LCOSB_VERBOSE 5

#define LCOSB_ECHO_DEBUG 15

// Basic pinout
#define MOTOR_PIN_L0      26
#define MOTOR_PIN_L1      25      
#define MOTOR_PIN_R0      33
#define MOTOR_PIN_R1      32

#define PWM_PIN_L         12
#define PWM_PIN_R         14

#define ECHO_PIN_0        15
#define ECHO_TRIGGER      2
#define ECHO_PIN_1        4

// ROVER DIMENSIONS
#define DIM_ROVER_LENGTH   2540
#define DIM_ROVER_BREATH   1524
#define DIM_ROVER_HEIGHT   1016

// NET
#define UNIT_IS_ROOT 0



//TODO: pin match/overlap check
//TODO: some visualization or cli  

#endif