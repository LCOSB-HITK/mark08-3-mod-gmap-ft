/*********
  author: Lucius Pertis
  Complete project details at  https://github.com/LCOSB-HITK/

  mark07-fieldtesting MOD_esp32d0wd
*********/

#include <WiFi.h>

#include "include/lcosb_lame.h"
#include "include/lcosb_echo.h"
#include "include/lcosb_motor.h"

#include "include/mark08_ft_httpd.h"

#include "include/lcosb_log.h"

const char* ssid = "LPn9";
const char* password = "pi=3.14159";

const char* serverURL = "http://your-server-ip:your-port"; // Replace with your server's IP and port

unsigned long currtime = 0;
int SCHEDULER_LOOP_1S = 0;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.print("Serving at ");
  Serial.println(WiFi.localIP());

  SetupMotorControls();
  Serial.println(">> Motor Controls init complete.");

  SetupEcho();
  initEchoBuff();
  Serial.println(">> Echo init complete.");

  _reCalcTraj();
  Serial.println(">> LAME init-upd chk complete.");

  StartHTTPDaemon();

  Serial.println("\nALL Modules started successfully.\nServer Started.\n");
  currtime = millis();
}

void loop() {
    delay(1000);
    SCHEDULER_LOOP_1S += 1;
    
    // 2 sec scheduled task
    if (SCHEDULER_LOOP_1S % 2 == 0)
    {
        inertDecaySteer();
    } else

    // 3 sec scheduled task
    if (SCHEDULER_LOOP_1S % 3 == 0)
    {
        inertDecayPower();
        sendLogsOverHttpClient();
    } else
    
    // 5 sec scheduled task
    if (SCHEDULER_LOOP_1S % 5 == 0)
    {
        printEchoBuffState();
    } else
    
    // 11 sec scheduled task
    if (SCHEDULER_LOOP_1S % 11 == 0)
    {
        
        char digest[100];
        int sd_f = get_sys_digest(digest, 100);
        Serial.print(">> Schedule system digest:");
        Serial.println(sd_f);
        if(sd_f != -1) { // degest creation success
            
            Serial.println(digest);
            unsigned long ctime;
            int stat[10];
            sscanf(digest, "%lu %d %d %d %d %d %d %d %d %d %d\n\0",
                        &ctime,
                        &stat[0], &stat[1], &stat[2], &stat[3], &stat[4], &stat[5],
                        &stat[6], &stat[7], &stat[8], &stat[9]);
            Serial.print(">>> ctime:\t");               Serial.println(ctime);
            Serial.print(">>> gpos_x:\t");              Serial.println(stat[0]);
            Serial.print(">>> gpos_y:\t");              Serial.println(stat[1]);
            Serial.print(">>> gpos_d:\t");              Serial.println(stat[2]);
            Serial.print(">>> gvel_v:\t");              Serial.println(stat[3]);
            Serial.print(">>> gvel_omega:\t");          Serial.println(stat[4]);
            Serial.print(">>> gvel_rcurve:\t");         Serial.println(stat[5]);
            Serial.print(">>> motorspeed[left] :\t");   Serial.println(stat[6]);
            Serial.print(">>> motorspeed[right]:\t");   Serial.println(stat[7]);
            Serial.print(">>> echo left :\t");          Serial.println(stat[8]);
            Serial.print(">>> echo right:\t");          Serial.println(stat[9]);
        }
    } else
    
    // 16 sec scheduled task
    if (SCHEDULER_LOOP_1S % 23 == 0)
    {
        
        echoBuffClean();
        Serial.println(">> echoBuffClean() complete.");
    }    
}
