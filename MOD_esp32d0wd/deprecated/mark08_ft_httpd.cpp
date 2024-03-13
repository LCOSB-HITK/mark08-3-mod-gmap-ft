/*********
  author: Lucius Pertis
  Complete project details at  https://github.com/LCOSB-HITK/

  mark08-fieldtesting MOD_esp32d0wd
*********/

#include "include/mark08_ft_httpd.h"

#include "include/lcosb_echo.h"
#include "include/lcosb_motor.h"
#include "include/lcosb_lame.h"

#include "include/lcosb_log.h"

#include <Arduino.h>


httpd_handle_t esp32d0wd_httpd = NULL;
esp_http_client_handle_t inf_client = NULL;


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
    lcosb_echo_t curr_echo;
    { // Debug logs
        Serial.print("Echo Record [DEF]: ctime=");
        Serial.print(curr_echo.ctime);
        Serial.print(", left=");
        Serial.print(curr_echo.left);
        Serial.print(", right=");
        Serial.println(curr_echo.right); 
        }

    recordEcho(&curr_echo);
    { // Debug logs
        Serial.print("Echo Record [FETCH]: ctime=");
        Serial.print(curr_echo.ctime);
        Serial.print(", left=");
        Serial.print(curr_echo.left);
        Serial.print(", right=");
        Serial.println(curr_echo.right);

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
                curr_echo.ctime,
                curr_gpos[0], curr_gpos[1], curr_gpos[2],
                curr_gvel[0], curr_gvel[1], curr_gvel[2],
                curr_motor[0], curr_motor[1]
                ,curr_echo.left, curr_echo.right
                );

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

void sendLogsOverHttpClient() {
    // Create a dynamic char buffer
    int bufferSize = 1024*1;  // 2 KB
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

    Serial.println("getLogs success:");
    Serial.print("numLogs: "); Serial.print(numLogs);
    Serial.print("  logsSize: "); Serial.println(logsSize);

    if (numLogs > 0) {
        // Set the buffer as the post data
        esp_http_client_set_post_field(inf_client, buffer, logsSize);
        esp_http_client_set_header(inf_client, "Content-Length", String(logsSize).c_str());

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

// debug-testing
// Function to handle the URI "/moveRover"
esp_err_t moveRover_handler(httpd_req_t *req) { // may have side effects (network-stack); requires POST
    {   // Debug logs
        Serial.println("Handling moveRover request...");
    }

    set_response_headers(req);

    char buf[50];  // Adjust buffer size accordingly

    // Check if the request method is POST
    if (req->method != HTTP_POST) {
        {   // Debug logs
            Serial.println("Invalid request method. Expected POST.");
        }
        httpd_resp_send_err(req, HTTPD_405_METHOD_NOT_ALLOWED, "POST method url");
        return ESP_OK;
    }

    // Read content of the POST request
    int content_length = req->content_len;
    if (content_length > sizeof(buf)) {
        {   // Debug logs
            Serial.println("Payload length exceeds limit.");
        }
        httpd_resp_send_err(req, HTTPD_414_URI_TOO_LONG, "payload length limit 50");
        return ESP_OK;
    }

    int ret = httpd_req_recv(req, buf, content_length);
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            {   // Debug logs
                Serial.println("Request timeout.");
            }
            httpd_resp_send_408(req);
        }
        {   // Debug logs
            Serial.println("Failed to receive request content.");
        }
        return ESP_FAIL;
    }

    // Null-terminate the received data
    buf[ret] = '\0';

    // Debug log for received data
    {   // Debug logs
        Serial.print("Received data: ");
        Serial.println(buf);
    }

    // Parse the received data for parameters (pow and steer)
    char *pow_param = strstr(buf, "pow=");
    char *steer_param = strstr(buf, "steer=");

    if (pow_param != NULL && steer_param != NULL) {
        {   // Debug logs
            Serial.println("Parsing parameters...");
        }

        int pow_value, steer_value;
        sscanf(pow_param, "pow=%d", &pow_value);
        sscanf(steer_param, "steer=%d", &steer_value);

        // Debug logs for parsed values
        {   // Debug logs
            Serial.print("Parsed pow: ");
            Serial.println(pow_value);
            Serial.print("Parsed steer: ");
            Serial.println(steer_value);
        }

        // TODO: logic-validate pow, steer

        // Debug log before forwarding commands
        {   // Debug logs
            Serial.println("Forwarding commands to motor...");
        }

        // Process forward steer to motor.h
        steerRover(pow_value, steer_value);
        //updateMotorOutput();

        {   // Debug logs
            Serial.println("Command received successfully.");
        }

        httpd_resp_send(req, "Command received successfully", HTTPD_RESP_USE_STRLEN);
    } else {
        {   // Debug logs
            Serial.println("Failed to parse parameters.");
        }
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "parse fail");
    }

    {   // Debug logs
        Serial.println("Request handling completed.");
    }

    return ESP_OK;
}

// debug-testing
esp_err_t ctrlMotor_handler(httpd_req_t *req) { // WILL NOT have side effects (network-stack)
    {   // Debug logs
        Serial.println("Handling ctrlMotor request...");
    }

    set_response_headers(req);

    char buf[50];  // Adjust buffer size accordingly

    // Check if the request method is GET
    if (req->method != HTTP_POST) {
        {   // Debug logs
            Serial.println("Invalid request method. Expected POST.");
        }
        httpd_resp_send_err(req, HTTPD_405_METHOD_NOT_ALLOWED, "POST method url");
        return ESP_OK;
    }

    // Read content of the GET request
    int content_length = req->content_len;
    {   // Debug logs
        Serial.print("Content length: ");
        Serial.println(content_length);
    }

    if (content_length > sizeof(buf)) {
        {   // Debug logs
            Serial.println("Payload length exceeds limit.");
        }
        httpd_resp_send_err(req, HTTPD_414_URI_TOO_LONG, "payload length limit 50");
        return ESP_OK;
    }

    int ret = httpd_req_recv(req, buf, content_length);
    {   // Debug logs
        Serial.print("Received bytes: ");
        Serial.println(ret);
    }

    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            {   // Debug logs
                Serial.println("Request timeout.");
            }
            httpd_resp_send_408(req);
        }
        {   // Debug logs
            Serial.println("Failed to receive request content.");
        }
        return ESP_FAIL;
    }

    // Null-terminate the received data
    buf[ret] = '\0';

    // Debug log for received data
    {   // Debug logs
        Serial.print("Received data: ");
        Serial.println(buf);
    }

    // Parse the received data for parameters (left_m and right_m)
    char *left_m_param = strstr(buf, "l_m=");
    char *right_m_param = strstr(buf, "r_m=");

    if (left_m_param != NULL && right_m_param != NULL) {
        {   // Debug logs
            Serial.println("Parsing parameters...");
        }

        int l_m_v, r_m_v;
        sscanf(left_m_param, "l_m=%d", &l_m_v);
        sscanf(right_m_param, "r_m=%d", &r_m_v);

        // Debug logs for parsed values
        {   // Debug logs
            Serial.print("Parsed left_m: ");
            Serial.println(l_m_v);
            Serial.print("Parsed right_m: ");
            Serial.println(r_m_v);
        }

        // TODO: logic-validate

        // Debug log before forwarding commands
        {   // Debug logs
            Serial.println("Forwarding commands to motor...");
        }

        // Process forward steer to motor.h
        setMotorSpeed(l_m_v, r_m_v);
        updateMotorOutput();

        {   // Debug logs
            Serial.println("Command received successfully.");
        }

        httpd_resp_send(req, "Command received successfully", HTTPD_RESP_USE_STRLEN);
    } else {
        {   // Debug logs
            Serial.println("Failed to parse parameters.");
        }
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "parse error");
    }

    {   // Debug logs
        Serial.println("Request handling completed.");
    }

    return ESP_OK;
}

// Function to handle the URI "/stopRover"
esp_err_t stopRover_handler(httpd_req_t *req) { // WILL NOT have side effects (network-stack)
    set_response_headers(req);

    motorKill();
    
    const char *response = "GET request received!";
    httpd_resp_send(req, response, strlen(response));

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
httpd_uri_t move_rover_uri = {
    .uri       = "/moveRover",
    .method    = HTTP_POST,
    .handler   = moveRover_handler,
    .user_ctx  = NULL
};
httpd_uri_t ctrl_motor_uri = {
    .uri       = "/ctrlMotor",
    .method    = HTTP_POST,
    .handler   = ctrlMotor_handler,
    .user_ctx  = NULL
};
httpd_uri_t stop_rover_uri = {
    .uri       = "/stopRover",
    .method    = HTTP_GET,
    .handler   = stopRover_handler,
    .user_ctx  = NULL
};
httpd_uri_t status_uri = {
    .uri       = "/status",
    .method    = HTTP_GET,
    .handler   = status_handler,
    .user_ctx  = NULL
};

void StartHTTPDaemon() {

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    Serial.print(">> Starting esp32d0wd_httpd web server on port:");
    Serial.println(config.server_port);

    if (httpd_start(&esp32d0wd_httpd, &config) == ESP_OK) {
        Serial.println(">> esp32d0wd_httpd config success");
        
        httpd_register_uri_handler(esp32d0wd_httpd, &root_uri);
        httpd_register_uri_handler(esp32d0wd_httpd, &move_rover_uri);
        httpd_register_uri_handler(esp32d0wd_httpd, &ctrl_motor_uri);
        httpd_register_uri_handler(esp32d0wd_httpd, &stop_rover_uri);
        httpd_register_uri_handler(esp32d0wd_httpd, &status_uri);
        
        Serial.println(">> esp32d0wd_httpd uri binding success");
    } else {
        Serial.println(">>>> ERR HTTPD");
    }

    esp_http_client_config_t config_c = {
        .url = "http://192.168.138.251:5000/log_text",
        .method = HTTP_METHOD_POST,
    };
    inf_client = esp_http_client_init(&config_c);
    esp_http_client_set_header(inf_client, "Content-Type", "text/plain");
}