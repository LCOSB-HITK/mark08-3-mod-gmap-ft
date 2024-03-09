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

#include "lcosb_netintf.h"



int pingIp(uint8_t* ipaddr);

int get_sys_digest(char* msgbuff, int size);
esp_http_client_handle_t initHttpClient(const char* url);
void StartHTTPDaemon();


#endif