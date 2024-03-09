/*********
  author: Lucius Pertis
  Complete project details at  https://github.com/LCOSB-HITK/

  mark08-fieldtesting MOD_esp32d0wd
*********/

#include "mark08_ft_httpd.h"

#include "lcosb_echo.h"
#include "lcosb_motor.h"
#include "lcosb_lame.h"
#include "lcosb_log.h"

// debug-testing
int get_sys_digest(char* msgbuff, int size) {
    int curr_gpos[3], curr_gvel[3], curr_motor[2];

    { // Debug logs
        Serial.println("Getting position..."); }
    getGPos(curr_gpos);
    { // Debug logs
        Serial.print("Position: ");
        Serial.print(curr_gpos[0]);
        Serial.print(", ");
        Serial.print(curr_gpos[1]);
        Serial.print(", ");
        Serial.println(curr_gpos[2]); 

        Serial.println("Getting velocity..."); }
    getGVel(curr_gvel);
    { // Debug logs
        Serial.print("Velocity: ");
        Serial.print(curr_gvel[0]);
        Serial.print(", ");
        Serial.print(curr_gvel[1]);
        Serial.print(", ");
        Serial.println(curr_gvel[2]);

        Serial.println("Recording echo..."); }
    echo_record_t curr_echo;
    { // Debug logs
        Serial.print("Echo Record [DEF]: stime=");
        Serial.print(curr_echo.stime);
        Serial.print(", d_l=");
        Serial.print(curr_echo.d_l);
        Serial.print(", d_r=");
        Serial.println(curr_echo.d_r); }

    recordEcho(&curr_echo);
    { // Debug logs
        Serial.print("Echo Record [FETCH]: stime=");
        Serial.print(curr_echo.stime);
        Serial.print(", d_l=");
        Serial.print(curr_echo.d_l);
        Serial.print(", d_r=");
        Serial.println(curr_echo.d_r);

        Serial.println("Getting motor speeds..."); }
    curr_motor[0] = getMotorSpeed(0);
    curr_motor[1] = getMotorSpeed(1);
    { // Debug logs
        Serial.print("Motor Speeds: ");
        Serial.print(curr_motor[0]);
        Serial.print(", ");
        Serial.println(curr_motor[1]);

        Serial.println("Formatting message..."); 
        Serial.print("sizeof(msgbuff):");
        Serial.println(sizeof(msgbuff));        }

    int cw = snprintf(
                msgbuff, size,
                "%lu %d %d %d %d %d %d %d %d %d %d\n\0",
                curr_echo.stime,
                curr_gpos[0], curr_gpos[1], curr_gpos[2],
                curr_gvel[0], curr_gvel[1], curr_gvel[2],
                curr_motor[0], curr_motor[1],
                curr_echo.d_l, curr_echo.d_r);

    if (cw < 0 || cw >= size) {
        { // Debug logs
            Serial.print("Error in formatting message; cw:"); 
            Serial.println(cw); }
        return -1;
    } else {
        { // Debug log
            Serial.print("Message formatted successfully. Length: ");
            Serial.println(cw); }
        return cw;
    }
}

void sendLogsOverHttpClient(esp_http_client_handle_t inf_client) {
    // Create a dynamic char buffer
    int bufferSize = 1024*2;  // 2 KB
    char* buffer = (char*)malloc(bufferSize);

    if (buffer == NULL) {
        // Handle memory allocation failure
        Serial.println("Error: Failed to allocate memory for buffer");
        return;
    }

    // Get logs into the buffer
    int logsSize = 0;
    int maxLogs = 10;  // Adjust the number of logs to fetch
    int numLogs = getLogs(buffer, &logsSize, maxLogs, bufferSize);

    if (numLogs > 0) {
        // Set the buffer as the post data
        esp_http_client_set_post_field(inf_client, buffer, logsSize);

        // Debug log: Show the logs in the buffer
        Serial.print("Logs in to send: ");
        for (int i = 0; i < logsSize; ++i) {
            Serial.print(buffer[i]);
        }
        Serial.println();

        // Debug log: Show the number of logs and their total size
        Serial.print("Number of logs: ");
        Serial.println(numLogs);
        Serial.print("Total size of logs: ");
        Serial.println(logsSize);

        // Send logs over the client connection
        esp_err_t err = esp_http_client_perform(inf_client);

        if (err == ESP_OK) {
            int response_code = esp_http_client_get_status_code(inf_client);
            // Debug log: Show the HTTP response code
            Serial.print("HTTP Response Code: ");
            Serial.println(response_code);
        } else {
            // Debug log: Show HTTP request error
            Serial.print("Error in HTTP request: ");
            Serial.println(esp_err_to_name(err));
        }
    } else {
        // Debug log: No logs to send
        Serial.println("No logs to send");
    }

    // Free the buffer
    free(buffer);
}

void set_response_headers(httpd_req_t *req) {
    // CORS headers
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, POST, PUT, DELETE");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Credentials", "true");
    httpd_resp_set_hdr(req, "Access-Control-Expose-Headers", "Content-Length, X-Content-Range");
}



// Function to handle the root URI "/"
esp_err_t root_handler(httpd_req_t *req) {
    set_response_headers(req);
    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}
// Function to handle the URI "/status"
esp_err_t status_handler(httpd_req_t *req) {
    set_response_headers(req);
    
    char buff[100];
    int sd_f = get_sys_digest(buff, 100);
    
    if(sd_f==-1) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "digest error");
        return ESP_FAIL;
    }

    httpd_resp_send(req, buff, strlen(buff));
    return ESP_OK;
}

// Register the URI handlers
httpd_uri_t root_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = root_handler,
    .user_ctx  = NULL
};
httpd_uri_t status_uri = {
    .uri       = "/status",
    .method    = HTTP_GET,
    .handler   = status_handler,
    .user_ctx  = NULL
};

httpd_handle_t esp32d0wd_httpd = NULL;
esp_http_client_handle_t inf_client = NULL;

void StartHTTPDaemon() {

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    Serial.print(">> Starting esp32d0wd_httpd web server on port:");
    Serial.println(config.server_port);

    if (httpd_start(&esp32d0wd_httpd, &config) == ESP_OK) {
        Serial.println(">> esp32d0wd_httpd config success");
        
        httpd_register_uri_handler(esp32d0wd_httpd, &root_uri);
        httpd_register_uri_handler(esp32d0wd_httpd, &status_uri);
        
        Serial.println(">> esp32d0wd_httpd uri binding success");
    } else {
        Serial.println(">>>> ERR HTTPD");
    }

    esp_http_client_config_t config = {
        .url = serverURL,
    };
    inf_client = esp_http_client_init(&config);
}