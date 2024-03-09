/***
  author: Lucius Pertis
  Complete project details at https://github.com/LCOSB-HITK/
***/


#include "include/lcosb_lame.h"
#include "include/lcosb_motor.h"

#define SIGN(x) ((x>=0)? 1:-1)
#define iABS(x) ((x) >= 0 ? (x) : -(x))

#define LAME_MAX_GVEL 72 // milli per sec
#define LAME_SPEED_VEL_FACT 7.2 // LAME_MAX_GVEL/MOTOR_SPEED_MAX; REFACTOR: 72/8 = 9

int gpos_x = 0;    // milli meter
int gpos_y = 0;    // milli meter
int gpos_d = 1570; // milli radians; range[0, 3141.5*2=6283]

// choosing polar representation for easy calc
double gvel_v = 0;        // milli meter per sec
double gvel_omega = 0;    // radian per sec
double gvel_rcurve = 0;   // milli meter

int gpos_x_err = 0;
int gpos_y_err = 0;
int gpos_d_err = 0;

int gpos_x_accerr_persec = 0;
int gpos_y_accerr_persec = 0;
int gpos_d_accerr_persec = 0;

unsigned long last_gpos_upd = 0; // millisec
unsigned long last_gvel_upd = 0; // millisec

double fast_atan(double x) {
    return x * (M_PI / 4.0) - x * (fABS(x) - 1) * (0.2447 + 0.0663 * fABS(x));
}

// debug-testing
void getGPos(int* gpos) {
    {   // Debug logs
        Serial.println("Getting global position...");
    }

    // TODO: validate gpos size
    unsigned long ctime = millis();

    double time_millis = ctime - last_gpos_upd;
    double local_dx = 0, local_dy = 0, theta = 0;
    double local_d_mag = 0;

    {   // Debug logs
        Serial.print("Time Millis: ");
        Serial.println(time_millis);
    }

    if (gvel_omega != 0) {
        {   // Debug logs
            Serial.println("Omega is NOT zero. Calculating local_d by polar params ...");
        }

        theta = gvel_omega * time_millis; // milli rads

        // local_dx = gvel_rcurve * sin(theta / 1000);
        // local_dy = gvel_rcurve * (1 - cos(theta / 1000));

        local_d_mag = 2 * gvel_rcurve * sin(theta / 1000 / 2);
    } else {
        {   // Debug logs
            Serial.println("Omega is zero. Calculating local_d by linear params...");
        }

        // local_dx = gvel_v * time_millis / 1000;
        // local_dy = gvel_v * time_millis / 1000;

        local_d_mag = gvel_v * time_millis / 1000;
    }

    {   // Debug logs
        Serial.print("Local DX: ");
        Serial.println(local_dx);
        Serial.print("Local DY: ");
        Serial.println(local_dy);
        Serial.print("Local D MAG SQ: ");
        Serial.println(local_d_mag);
        Serial.print("Theta: ");
        Serial.println(theta);
    }

    gpos[0] = gpos_x + local_d_mag * cos((double)(gpos_d - theta) / 1000);
    gpos[1] = gpos_y + local_d_mag * sin((double)(gpos_d - theta) / 1000);

    gpos[2] = gpos_d + theta;

    {   // Debug logs
        Serial.print("Updated GPos[0] / x: ");
        Serial.println(gpos[0]);
        Serial.print("Updated GPos[1] / y: ");
        Serial.println(gpos[1]);
        Serial.print("Updated GPos[2] / d: ");
        Serial.println(gpos[2]);
        Serial.println("Global position retrieval completed.");
    }
}

void _getGPos(int* gpos) {
    //TODO: validate gpos size
    unsigned long ctime = millis();

    double time_millis = ctime - last_gpos_upd;
    double local_dx=0, local_dy=0, theta=0;

    if (gvel_omega==0) {
        theta = gvel_omega*time_millis; // milli rads

        local_dx = gvel_rcurve*   sin(theta/1000);
        local_dy = gvel_rcurve*(1-cos(theta/1000));

    } else {
        local_dx = gvel_v*time_millis/1000;
        local_dy = gvel_v*time_millis/1000;
        
    }

    gpos[0] = gpos_x + local_dx*cos((double)gpos_d/1000);
    gpos[1] = gpos_y + local_dy*sin((double)gpos_d/1000);

    gpos[2] = gpos_d + theta;
}

void getGVel(int* gvel) {
    //TODO:  validate gvel size
    gvel[0] = (int) gvel_v;
    gvel[1] = (int) gvel_omega*1000; // milli rad per sec
    gvel[2] = (int) gvel_rcurve;     // millimeter
}

void setGPos(int pos[3]) {
    //TODO: force validate pos values before assignment
    gpos_x = pos[0];
    gpos_y = pos[1];
    gpos_d = pos[2]%6283;
}

void setGVel(double vel[3]) {
    //TODO: force validate pos values before assignment
    gvel_v = vel[0];
    gvel_omega = vel[1];
    gvel_rcurve = vel[2];
}

// debug-testing
void reCalcTraj() {
    {   // Debug logs
        Serial.println("Recalculating trajectory...");
    }

    updateGPos();

    double delta_speed = - (motorspeed[0] - motorspeed[1]) * LAME_SPEED_VEL_FACT; // right - left; if right is high theta increases
    double avg_speed = (motorspeed[0] + motorspeed[1]) * LAME_SPEED_VEL_FACT / 2;

    {   // Debug logs
        Serial.print("Motorspeed[0]: ");
        Serial.println(motorspeed[0]);
        Serial.print("Motorspeed[1]: ");
        Serial.println(motorspeed[1]);
        Serial.print("Delta Speed: ");
        Serial.println(delta_speed);
        Serial.print("Average Speed: ");
        Serial.println(avg_speed);
    }

    gvel_v = avg_speed;

    if (motorspeed[0] - motorspeed[1] == 0) {
        {   // Debug logs
            Serial.println("Motorspeeds are equal.");
        }
        gvel_omega = 0;
        gvel_rcurve = 1;
    } else {
        {   // Debug logs
            Serial.println("Motorspeeds are not equal. Calculating omega and rcurve...");
        }

        gvel_omega = fast_atan(delta_speed / DIM_ROVER_BREATH); // rad per sec
        gvel_rcurve = avg_speed * DIM_ROVER_BREATH / delta_speed; // millimeter

        {   // Debug logs
            Serial.print("Gvel Omega: ");
            Serial.println(gvel_omega);
            Serial.print("Gvel Rcurve: ");
            Serial.println(gvel_rcurve);
        }
    }

    last_gvel_upd = millis();

    {   // Debug logs
        Serial.print("Last Gvel Update: ");
        Serial.println(last_gvel_upd);
        Serial.println("Trajectory recalculation completed.");
    }
}

/**
 * @brief Recalculate angular and linear motion parameters based on motor speeds.
 *
 * This function recalculates the angular velocity and radius of curvature of a rover's motion
 * based on the speeds of its left and right motors. The calculated values are stored in global
 * variables for later use in position updates.
 *
 * @note This function assumes that the global variables motorspeed, LAME_SPEED_VEL_FACT,
 * DIM_ROVER_BREATH, gvel_omega, gvel_rcurve, and gvel_v are properly initialized and accessible.
 *
 * @details
 * - If the left and right motor speeds are equal, the rover is moving straight, and the angular
 *   velocity (gvel_omega) is set to 0, while the radius of curvature (gvel_rcurve) is set to 1.
 * - If there is a speed difference between the left and right motors, the angular velocity
 *   (gvel_omega) is calculated using the arctangent approximation of the ratio of the speed
 *   difference to the rover's breadth (DIM_ROVER_BREATH).
 * - The linear velocity (gvel_v) is set to the average speed of the left and right motors.
 * - The radius of curvature (gvel_rcurve) is calculated based on the average speed and the ratio of
 *   the average speed to the speed difference.
 *
 * @pre Ensure that the motorspeed array, LAME_SPEED_VEL_FACT, DIM_ROVER_BREATH, gvel_omega,
 * gvel_rcurve, and gvel_v are properly initialized and accessible.
 *
 * @post The global variables gvel_omega, gvel_rcurve, and gvel_v are updated based on the current
 * motor speeds.
 *
 * @see fast_atan() for the arctangent approximation used in the calculations.
 *
 * @warning This function may not handle extreme cases (e.g., very small speed differences) and
 * could potentially lead to division by zero or other numerical issues. Ensure proper error
 * handling and unit consistency in your application.
 */
void _reCalcTraj() {
    updateGPos();

    double delta_speed = (motorspeed[0] - motorspeed[1])*LAME_SPEED_VEL_FACT;
    double avg_speed   = (motorspeed[0] + motorspeed[1])*LAME_SPEED_VEL_FACT/2;

    gvel_v = avg_speed;

    if (motorspeed[0]-motorspeed[1]==0) {
        gvel_omega = 0;
        gvel_rcurve = 1;
    } else {
        gvel_omega = fast_atan(delta_speed/DIM_ROVER_BREATH); // rad per sec
        gvel_rcurve = avg_speed*DIM_ROVER_BREATH/delta_speed; // millimeter
    }

    last_gvel_upd = millis();
}

/** Inverse kinematics for path-calc
 * 
 * rvel_rcurve and rvel_avg are redundant/dual to each other
 * only for straight lines (omega==0), rvel_rcurve will be
 * undefined
 * 
 * Convention
 * - if (rvel_avg==0) force calc using (rvel_rcurve)
*/
void _invCalcTraj(int* calc_mspeed, double rvel_avg, double rvel_omega, double rvel_rcurve) {

    if (fABS(rvel_omega) < 0.001) {
        calc_mspeed[0] = rvel_avg/LAME_SPEED_VEL_FACT;
        calc_mspeed[1] = rvel_avg/LAME_SPEED_VEL_FACT;
        return;
    }

    double delta_speed = tan(rvel_omega)*DIM_ROVER_BREATH;

    if (fABS(rvel_avg) < 0.001)
        rvel_avg = delta_speed*rvel_rcurve/DIM_ROVER_BREATH;

    calc_mspeed[0] = int((rvel_avg + delta_speed/2)/LAME_SPEED_VEL_FACT);
    calc_mspeed[1] = int((rvel_avg - delta_speed/2)/LAME_SPEED_VEL_FACT); 
}

void lcosb_lame_moveGPos(int *gvel, int t, lcosb_gops_t *initpos, lcosb_gpos_t *finalpos) {
    if (gvel[1]==0) {
        finalpos[0] = gvel[0]*t + cp->initpos[0];
		finalpos[1] = gvel[1]*t + cp->initpos[1];
		finalpos[2] = initpos[2];
    }
    else {
        int theta = gvel[1]*t; // total change in theta
        
        int Rcurve=0;
        if (gvel[0]!=0) Rcurve = gvel[0]*100/gvel[1];   // calc via tangential vel
        else            Rcurve = gvel[2];               // calc via gvel.rcurve
    
        finalpos[0] = initpos[0] + Rcurve*cos(theta);
        finalpos[1] = initpos[1] - Rcurve*sin(theta);
        finalpos[2] = initpos[2] + theta;
    }
}

unsigned long lcosb_lame_getLastGvelUpdate() {
    return last_gvel_upd;
}

void getGPosErr(int *err) {
    err[0] = gpos_x_err;
    err[1] = gpos_y_err;
    err[2] = gpos_d_err;
}

void updateGPos() {
    int pos[3];
    getGPos(pos);
    setGPos(pos);

    last_gpos_upd = millis();
}
