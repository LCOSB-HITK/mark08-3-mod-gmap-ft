/***
  author: Lucius Pertis
  Complete project details at https://github.com/LCOSB-HITK/
***/


#include "lcosb_motor.h"
#include "lcosb_lame.h"

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

#define SIGN(x) (x>=0)? 1:-1
#define ABS(x) ((x) >= 0 ? (x) : -(x))

// PWM analog state for motors
#define PWM_CHANNEL_L     1
#define PWM_CHANNEL_R     2

// use 12 bit precission for LEDC timer
#define LEDC_TIMER_12_BIT  12
#define LEDC_DUTY_MAX_FLOAT_12_BIT 4095.0
#define PWM_BASE_FREQ     10000

// #define PWM_PIN_L         12
// #define PWM_PIN_R         14


// motor motor pins
// #define MOTOR_PIN_L0      5
// #define MOTOR_PIN_L1      18      
// #define MOTOR_PIN_R0      19
// #define MOTOR_PIN_R1      21

#define MOTOR_SPEED_MAX   10 //TODO: REFACTOR: 8
#define STEER_GRANULARITY 8
#define STEER_GRAIN_INV_SHIFT 3   //log2(STEER_GRANULARITY)

// mechtronic specific empirical behaviour 
#define MOTOR_DUTY_MIN_12_BIT 1966 // 48% of 4096


int motorspeed[2];   //speed of motor in range [-10, +10]
//TODO:
// typedef struct InertialDriveSystem {
//     int power;
//     int steer;

//     int powerdecay
// }

void SetupMotorControls(){
    // Setup timer and attach timer to a motor pins
    ledcSetup(PWM_CHANNEL_L, PWM_BASE_FREQ, LEDC_TIMER_12_BIT);
    ledcAttachPin(PWM_PIN_L, PWM_CHANNEL_L);
    ledcSetup(PWM_CHANNEL_R, PWM_BASE_FREQ, LEDC_TIMER_12_BIT);
    ledcAttachPin(PWM_PIN_R, PWM_CHANNEL_R);

    // Setup motor direction pins
    pinMode(MOTOR_PIN_L0, OUTPUT);
    pinMode(MOTOR_PIN_L1, OUTPUT);
    pinMode(MOTOR_PIN_R0, OUTPUT);
    pinMode(MOTOR_PIN_R1, OUTPUT);

    // Set internal motor state
    //motorspeed = {0, 0};
    motorKill();
}

// custom for motors
void pwmWriteMotors(uint8_t channel, float value, float valueMax) {
    uint32_t duty = MOTOR_DUTY_MIN_12_BIT + 
                    (LEDC_DUTY_MAX_FLOAT_12_BIT - MOTOR_DUTY_MIN_12_BIT) * min((value/valueMax), 1.0);

    ledcWrite(channel, duty);
}

void updateMotorOutput() {
    if (motorspeed[0] < 0)  { digitalWrite(MOTOR_PIN_L0, LOW);  digitalWrite(MOTOR_PIN_L1, HIGH); }
    else                    { digitalWrite(MOTOR_PIN_L0, HIGH); digitalWrite(MOTOR_PIN_L1, LOW);  }

    if (motorspeed[1] < 0)  { digitalWrite(MOTOR_PIN_R0, LOW);  digitalWrite(MOTOR_PIN_R1, HIGH); }
    else                    { digitalWrite(MOTOR_PIN_R0, HIGH); digitalWrite(MOTOR_PIN_R1, LOW);  }

    pwmWriteMotors(PWM_CHANNEL_L, abs(motorspeed[0]), MOTOR_SPEED_MAX);
    pwmWriteMotors(PWM_CHANNEL_R, abs(motorspeed[1]), MOTOR_SPEED_MAX);

    reCalcTraj();
}

void motorKill() {
    motorspeed[0] = 0;
    motorspeed[1] = 0;

    digitalWrite(MOTOR_PIN_L0, LOW);  digitalWrite(MOTOR_PIN_L1, LOW);
    digitalWrite(MOTOR_PIN_R0, LOW);  digitalWrite(MOTOR_PIN_R1, LOW);

    ledcWrite(PWM_CHANNEL_L, 0);
    ledcWrite(PWM_CHANNEL_R, 0);

    reCalcTraj();
}

// global temp vars (need better arch style)
int gpower = 0, gsteer = 0, last_max_power = 0;

void steerRover(int power_inc, int steer_inc) {
    // inertial drive kinematics to be implemented
    // rightnow just a simple quantized version will do
    // state needs to be maintained by client daemon
    // decay-delay will be handled here

    if ((abs(gpower + power_inc)+abs(gsteer + steer_inc) > MOTOR_SPEED_MAX)
         && gpower && gsteer ) { // CRITICAL Steer and NOT at criticality vertex (+-10,0)(0.+-10)
        //change power
        gpower += SIGN(power_inc);
        if (power_inc*gpower > 0) // inc in criticallity
            gsteer -= SIGN(power_inc);
        
        // change steer
        gsteer += SIGN(steer_inc);
        if (steer_inc*gsteer > 0) // inc in criticallity
            gpower -= SIGN(steer_inc);
        
        if ( abs(last_max_power) <= abs(gpower) )
            last_max_power = gpower;
    } 
    else { // non-critical steer
        gpower += power_inc;
        gsteer += steer_inc;
        
        last_max_power = gpower;
    }

    gpower = max(-MOTOR_SPEED_MAX, min(gpower,  MOTOR_SPEED_MAX));
    gsteer = max(-MOTOR_SPEED_MAX, min(gsteer,  MOTOR_SPEED_MAX));
    
    setMotorSpeed( gpower - gsteer*SIGN(gpower), gpower + gsteer*SIGN(gpower));

    updateMotorOutput();
}

void inertDecayPower() {
    if (gpower) steerRover(SIGN(gpower)*-1, 0);
}

void inertDecaySteer() {
    if (last_max_power == gpower) {
        // non critical steer
        if(gsteer) steerRover(0, SIGN(gsteer)*-1); }
    else
        // critical steer (implied)
        steerRover(SIGN(last_max_power), 0);
}

int moveRoverByDist(int dist, int tp) {
    int mspeed[2];
    _invCalcTraj(mspeed, (double)dist/tp, 0, 0);
		
    // hard chk if angle can be changed; its implied (mpeed[0]==mpeed[1])
    if ( mpeed[0] != 0 ) {	
        setMotorSpeed(mspeed[0], mspeed[1]);
        updateMotorOutput();

        return 0;
    }
    return -1;
}

int rotateRoverByAngle(int theta, int tp) {
    int mspeed[2];
    _invCalcTraj(mspeed, 0, (double)theta/tp, 0);
		
    // hard chk if angle can be changed; its implied (mpeed[0]==mpeed[1])
    if ( mpeed[0] != 0 ) {	
        setMotorSpeed(mspeed[0], mspeed[1]);
        updateMotorOutput();

        return 0;
    }
    return -1;
}

int getMotorSpeed(uint8_t right) {
    return motorspeed[right % 2];
}

void incMotorSpeed(uint8_t right, int8_t inc) {
    motorspeed[right % 2] += inc;
}

// avoid using this func
void setMotorSpeed(int left_speed, int right_speed) {
    motorspeed[0] = max(-MOTOR_SPEED_MAX, min(left_speed,  MOTOR_SPEED_MAX));
    motorspeed[1] = max(-MOTOR_SPEED_MAX, min(right_speed, MOTOR_SPEED_MAX));
}

void printStats() {
    //Serial.print("motorspeed[0::L] :"); Serial.println(String(motorspeed[0]));
    //Serial.print("motorspeed[1::R] :"); Serial.println(String(motorspeed[1]));
}
