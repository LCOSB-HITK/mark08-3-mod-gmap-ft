/*********
  author: Lucius Pertis
  Complete project details at  https://github.com/LCOSB-HITK/
*********/

//mark08_ft_httpd.h
#ifndef MARK08_FT_HTTPD
#define MARK08_FT_HTTPD

#include <Arduino.h>
#include <esp_err.h>

#include "esp_http_server.h"
#include "esp_http_client.h"


//extern httpd_handle_t esp32d0wd_httpd;
//const char* infURL = "http://your-server-ip:your-port";

int get_sys_digest(char* msgbuff, int size);
esp_http_client_handle_t initHttpClient(const char* url);
void sendLogsOverHttpClient();

void StartHTTPDaemon();


#endif