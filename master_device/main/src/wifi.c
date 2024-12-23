#include "include/wifi.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_interface.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "esp_now.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "include/button.h"
#include "include/components.h"
#include "include/data.h"
#include "include/nvs.h"
#include "include/task.h"
#include <stdint.h>
#include <string.h>
#include "esp_http_client.h"
#include "esp_tls.h"

#include "include/led.h"
#include "include/ntp.h"
#include "lwip/ip4_addr.h"
#include "portmacro.h"
#include "extlib/cJSON.h"

#define MAX_RETRIES_ESP_NOW 10

static const char *TAG_WIFI = "WIFI";

static esp_now_peer_info_t peer_list[NUMBER_OF_DEVICES];

static wifi_flags_t flags = {0};

static TimerHandle_t setup_timer = NULL;

static esp_netif_t *esp_netif_ap;
static esp_netif_t *esp_netif_sta;

static char url[64];
static char api_key[148];

static wifi_config_t wifi_sta_config = {
    .sta = {
        .scan_method = WIFI_ALL_CHANNEL_SCAN,
        .failure_retry_cnt = 10,
        .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
    },
};

wifi_config_t wifi_ap_config = {
    .ap = {
        .channel = 0,
        .max_connection = 1,
        .ssid_hidden = true,
        .authmode = WIFI_AUTH_WPA3_PSK,
        .pmf_cfg = {
            .required = true,
        },
    },
};

wifi_flags_t* get_wifi_flags(void)
{
    return &flags; 
}

// SUPPORT FUNCTIONS
void mac_64_str(const uint64_t u64, char *str)
{
    char buffer[3] = "\0";
    char string_buffer[18] = "\0";
    for (int i = 5; i >= 0; i--)
    {
        sprintf(buffer, "%02x", (uint8_t) ((u64 >> (i*8)) & 0xFF));
        strcat(string_buffer, buffer); 
        if (i > 0)
            strcat(string_buffer, ":"); 
             
    }
    string_buffer[17] = '\0'; 
    strcpy(str, string_buffer);
}


static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    // AP CONNECTED 
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) 
    {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *) event_data;
        ESP_LOGI(TAG_WIFI, "Station "MACSTR" joined, AID=%d",
                 MAC2STR(event->mac), event->aid);
        if (flags.setup_mode)
        {
            esp_wifi_deauth_sta(0);
        }
        flags.ap_connected = 1; 
    } 

    // AP DISCONNECTED
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) 
    {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *) event_data;
        ESP_LOGI(TAG_WIFI, "Station "MACSTR" left, AID=%d, reason:%d",
                 MAC2STR(event->mac), event->aid, event->reason);
        flags.ap_connected = 0; 
    }

    // STA STARTED 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
    {
        esp_wifi_connect();
        ESP_LOGI(TAG_WIFI, "Station started");
    } 

    // STA CONNECTED 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) 
    {
        ESP_LOGI(TAG_WIFI, "Station connected...");
        flags.sta_connected = 1;
    } 

    // STA DISCONNECTED
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
    {
        ESP_LOGI(TAG_WIFI, "Station disconnected...");
        flags.sta_connected = 0;
        my_sntp_deinit();
        esp_wifi_connect();
    } 

    // STA GOT IP
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(TAG_WIFI, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
        flags.got_ip = 1;
        my_sntp_init();
    }

    // STA LOST IP
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_LOST_IP) {
        ESP_LOGI(TAG_WIFI, "Lost IP:");
        flags.got_ip = 0;
    }
}


static esp_netif_t *wifi_init_ap(void)
{
    char ssid_ap[33]; 
    char psswd_ap[65];

    char serial[SERIAL_NUMBER_SIZE];
    uint8_t mac[6];
    uint64_t mac_address;

    get_master_device(serial, mac);
    mac_8_64(mac, &mac_address);
    mac_64_str(mac_address, psswd_ap);
    strcpy(ssid_ap, serial);

    strcpy((char *) wifi_ap_config.ap.ssid, ssid_ap);    
    strcpy((char *) wifi_ap_config.ap.password, psswd_ap);    

    wifi_ap_config.ap.ssid[31] = '\0';
    wifi_ap_config.ap.password[63] = '\0';

    wifi_ap_config.ap.max_connection = 1;

    ESP_LOGI(TAG_WIFI, "WIFI SETUP MODE = OFF");

    esp_netif_t *esp_netif_ap = esp_netif_create_default_wifi_ap();
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_config));

    esp_netif_dhcps_stop(esp_netif_ap);

    esp_netif_ip_info_t ip_info;
    IP4_ADDR(&ip_info.ip, 192, 168, 1, 100);
    IP4_ADDR(&ip_info.gw, 192, 168, 1, 1);
    IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);

    ESP_ERROR_CHECK(esp_netif_set_ip_info(esp_netif_ap, &ip_info));

    esp_netif_dhcps_start(esp_netif_ap);

    return esp_netif_ap;
}


static esp_netif_t *wifi_init_sta(void)
{

    char ssid[32] = "";
    char psswd[64] = "";

    ESP_ERROR_CHECK(get_wifi_sm_cred_from_nvs(ssid, psswd));
    strcpy((char *) wifi_sta_config.sta.ssid, ssid); 
    strcpy((char *) wifi_sta_config.sta.password, psswd);

   // For safety (casting char *)
    wifi_sta_config.sta.ssid[31] = '\0';
    wifi_sta_config.sta.password[63] = '\0';

    esp_netif_t *esp_netif_sta = esp_netif_create_default_wifi_sta();
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_sta_config));

    return esp_netif_sta;
}


static esp_err_t set_setup_mode(bool state)
{
    esp_err_t err = ESP_OK;

    if (flags.setup_mode != state)
    {
        flags.setup_mode = state;
    }

    return err;
}

esp_err_t start_setup_mode(void)
{
    esp_err_t err = ESP_OK;
    set_setup_mode(true);
    if (xTimerIsTimerActive(setup_timer))
    {
        ESP_LOGI(TAG_WIFI, "Timer reset. Time left = %d min", (MAX_SETUP_TIME / 60000));
        xTimerReset(setup_timer, 5);
    }
    else 
    {
        ESP_LOGI(TAG_WIFI, "Setup mode started. Time left = %d min", (MAX_SETUP_TIME / 60000));
        ESP_LOGI(TAG_WIFI, "SSID = %s", wifi_ap_config.ap.ssid);
        ESP_LOGI(TAG_WIFI, "PSSWD = %s", wifi_ap_config.ap.password);
        xTimerStart(setup_timer, 5);
    }
    return err;
}

esp_err_t stop_setup_mode(void)
{
    esp_err_t err = ESP_OK;
    set_setup_mode(false);
    xTimerStop(setup_timer, 5);
    return err;
}

void callback_setup_time_expired(TimerHandle_t xTimer)
{
    stop_setup_mode();
}


esp_err_t wifi_init()
{
    esp_err_t err = ESP_OK;

    if (!flags.wifi_initialized)
    {
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());

        /* Register Event handler */
        ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                        ESP_EVENT_ANY_ID,
                        &wifi_event_handler,
                        NULL,
                        NULL));
        ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                        IP_EVENT_STA_GOT_IP,
                        &wifi_event_handler,
                        NULL,
                        NULL));

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
        ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
        ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_APSTA));
        esp_netif_sta = wifi_init_sta();
        ESP_LOGI(TAG_WIFI, "WIFI SM INITIALIZATION...");
        esp_netif_ap = wifi_init_ap();
        ESP_LOGI(TAG_WIFI, "WIFI AP INITIALIZATION...");
        esp_netif_set_default_netif(esp_netif_sta);

        start_webserver();
        flags.wifi_initialized = 1;

        if (setup_timer == NULL)
        {
            button_init();
            setup_timer = xTimerCreate(
                "Setup timer", 
                pdMS_TO_TICKS(MAX_SETUP_TIME), 
                pdFALSE, 
                (void*)0, 
                callback_setup_time_expired);
        }
    }
    return err;
}


static esp_err_t peer_list_setup(void)
{
    esp_err_t err = ESP_OK;
    int peers_count = 0;

    slave_device_t device_list[NUMBER_OF_DEVICES];
    get_slave_devices(device_list);


    for (int i = 0; i < NUMBER_OF_DEVICES; i++)
    {
        uint64_t mac;
        mac_8_64(device_list[i].mac_address, &mac);
        ESP_LOGI(TAG_WIFI, "DEVICE: id=%d, active=%d, serial=%s, mac=%" PRIu64, i, device_list[i].active, device_list[i].serial_number, mac);
    }

    for (int i = 0; i < NUMBER_OF_DEVICES; i++)
    {
        if (device_list[i].active)
        {
            peer_list[peers_count].ifidx = WIFI_IF_STA;
            memcpy(peer_list[peers_count].peer_addr, device_list[i].mac_address, sizeof(device_list[i].mac_address));
            esp_now_add_peer(&peer_list[peers_count]);
            peers_count++;
        }
    }
    return err;
}

static EventGroupHandle_t esp_now_evt_group;

static void esp_now_recv_cb(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len)
{
    static recv_data_t recieve_data;

    ESP_LOGI(TAG_WIFI, "%d bytes incoming from " MACSTR, len, MAC2STR(recv_info->src_addr));

    if(len != sizeof(recieve_data))
    {
        ESP_LOGE(TAG_WIFI, "Unexpected data length: %d != %u", len, (int) sizeof(recv_data_t));
        return;
    }

    memcpy(&recieve_data, data, len);

    if (*recieve_data.mac_address != *recv_info->src_addr)
    {
        ESP_LOGI(TAG_WIFI, "ESP-NOW recieve data error. MAC does not match");
        return;
    }

    if (xQueueSend(get_rcv_data_handle(), &recieve_data, 0) != pdTRUE) {
        ESP_LOGW(TAG_WIFI, "Queue full, discarded");
        return;
    }

}

static void esp_now_sent_cb(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    if (mac_addr == NULL) {
        ESP_LOGE(TAG_WIFI, "ESP-NOW send data error...");
        return;
    }
    xEventGroupSetBits(esp_now_evt_group, BIT(status));
}

esp_err_t my_esp_now_init(void)
{
    esp_err_t err = ESP_OK;

    if (!flags.esp_now_initiated && !flags.setup_mode)
    {
        ESP_ERROR_CHECK(esp_now_init());
        ESP_LOGI(TAG_WIFI, "ESP_NOW INITIALIZATION...");

        peer_list_setup();
           
        esp_now_evt_group = xEventGroupCreate();

        ESP_ERROR_CHECK(esp_now_register_recv_cb(esp_now_recv_cb));
        ESP_ERROR_CHECK(esp_now_register_send_cb(esp_now_sent_cb));

        flags.esp_now_initiated = 1;
    }
    return err;
}


 esp_err_t send_espnow_data(send_data_t data)
{
    esp_err_t err = ESP_OK;

    // Peer validation
    if (!esp_now_is_peer_exist(data.mac_address))
    {
        ESP_LOGI(TAG_WIFI, "Error, peer does not exists" MACSTR, MAC2STR(data.mac_address));
        err = ESP_NOW_SEND_FAIL;
        return err;
    }

    // Send it
    ESP_LOGI(TAG_WIFI, "Sending data to " MACSTR, MAC2STR(data.mac_address));

    uint8_t primary = 0;
    wifi_second_chan_t secondary;
    esp_wifi_get_channel(&primary, &secondary);
    data.channel = primary;

    int number_of_retries = 0;

    while ( number_of_retries < MAX_RETRIES_ESP_NOW)
    {
        err = esp_now_send(data.mac_address, (uint8_t*)&data, sizeof(data));
        if(err != ESP_OK)
        {
            ESP_LOGE(TAG_WIFI, "Send error (%s)", esp_err_to_name(err));
            return err;
        }

        // Wait for callback function to set status bit
        EventBits_t bits = xEventGroupWaitBits(
            esp_now_evt_group, 
            BIT(ESP_NOW_SEND_SUCCESS) | BIT(ESP_NOW_SEND_FAIL), 
            pdTRUE, 
            pdFALSE, 
            2000 / portTICK_PERIOD_MS);

        if ( !(bits & BIT(ESP_NOW_SEND_SUCCESS)) )
        {
            if (bits & BIT(ESP_NOW_SEND_FAIL))
            {
                err = ESP_FAIL;
            }
            else
            {
                err = ESP_ERR_TIMEOUT;
            }
        }
        else
        {
            err = ESP_OK;
            ESP_LOGI(TAG_WIFI, "Data sent!\n");
            blink_signal_led();
            return err;
        }
        ++number_of_retries; 
    }

    if (err != ESP_OK)    
        ESP_LOGE(TAG_WIFI, "ESP-NOW send error (%s)", esp_err_to_name(err));

    ESP_LOGI(TAG_WIFI, "Number of retries = %d\n", number_of_retries);
    return ESP_OK;
}


// HTTP
#define MAX_HTTP_OUTPUT_BUFFER 2048

esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    static char *output_buffer;  // Buffer to store response of http request from event handler
    static int output_len;       // Stores number of bytes read
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGE(TAG_WIFI, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            break;
        case HTTP_EVENT_HEADER_SENT:
            break;
        case HTTP_EVENT_ON_HEADER:
            break;
        case HTTP_EVENT_ON_DATA:
            // Clean the buffer in case of a new request
            if (output_len == 0 && evt->user_data) {
                // we are just starting to copy the output data into the use
                memset(evt->user_data, 0, MAX_HTTP_OUTPUT_BUFFER);
            }
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // If user_data buffer is configured, copy the response into the buffer
                int copy_len = 0;

                if (evt->user_data) {
                    evt->data_len < (MAX_HTTP_OUTPUT_BUFFER - output_len) ? copy_len = evt->data_len : (MAX_HTTP_OUTPUT_BUFFER - output_len);
                    if (copy_len) {
                        memcpy(evt->user_data + output_len, evt->data, copy_len);
                    }
                } else {
                    int content_len = esp_http_client_get_content_length(evt->client);
                    if (output_buffer == NULL) {
                        output_buffer = (char *) calloc(content_len + 1, sizeof(char));
                        output_len = 0;
                        if (output_buffer == NULL) {
                            ESP_LOGE(TAG_WIFI, "Failed to allocate memory for output buffer");
                            return ESP_FAIL;
                        }
                    }
                    evt->data_len < (content_len - output_len) ? copy_len = evt->data_len : (content_len - output_len);
                    if (copy_len) {
                        memcpy(output_buffer + output_len, evt->data, copy_len);
                    }
                }
                output_len += copy_len;
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            if (output_buffer != NULL) {
                free(output_buffer);
                output_buffer = NULL;
            }
            output_len = 0;
            break;
        case HTTP_EVENT_DISCONNECTED:
            if (output_buffer != NULL) {
                free(output_buffer);
                output_buffer = NULL;
            }
            output_len = 0;
            break;
        case HTTP_EVENT_REDIRECT:
            ESP_LOGI(TAG_WIFI, "HTTP_EVENT_REDIRECT");
            break;
    }
    return ESP_OK;
}


static int decode_json_and_save(const char *strJSON)
{
    char new_serial[SERIAL_NUMBER_SIZE];
    light_control_t new_data;

    const cJSON *serial_number = NULL;
    const cJSON *light_mode = NULL;
    const cJSON *light_value = NULL;

    slave_device_t slaves[NUMBER_OF_DEVICES];
    get_slave_devices(slaves);

    int status = 0;
    cJSON *str = cJSON_Parse(strJSON);
    if (str == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            ESP_LOGE(TAG_WIFI, "Error before %s\n", error_ptr);
        }
        status = 0;
        goto end;
    }

    for (int i = 0; i < NUMBER_OF_DEVICES; i++)
    {
        serial_number = cJSON_GetObjectItemCaseSensitive(str, slaves[i].serial_number);
        if (serial_number != NULL)
        {
            strcpy(new_serial, serial_number->string);

            light_mode = cJSON_GetObjectItemCaseSensitive(serial_number, "light_mode");
            new_data.auto_light = light_mode->valueint;

            light_value = cJSON_GetObjectItemCaseSensitive(serial_number, "light_value");
            new_data.light_value = light_value->valueint;

            set_light_data(i, serial_number->string, new_data);
        }
    }

end:
    cJSON_Delete(str);
    return status;
}


void set_http_cred(char *url_new, char *api_key_new)
{
    strcpy(url, url_new);
    strcpy(api_key, api_key_new);
}

            
esp_err_t send_data_to_db(char *string_JSON)
{
    char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER + 1] = {0};

    esp_err_t err = ESP_OK;
    esp_http_client_config_t config = {
        .host = "localhost",
        .path = "/post",
        .query = "esp",
        .user_data = local_response_buffer,
        .transport_type = HTTP_METHOD_POST,
        .event_handler = http_event_handler,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

// POST
    esp_http_client_set_url(client, url);
    esp_http_client_set_authtype(client, HTTP_AUTH_TYPE_BASIC);
    esp_http_client_set_post_field(client, string_JSON, strlen(string_JSON));
    esp_http_client_set_header(client, "Authorization", api_key);
    esp_http_client_set_header(client, "Content-Type", "application/json");

    esp_http_client_set_method(client, HTTP_METHOD_POST);

    ESP_LOGI(TAG_WIFI, "Posting data to database");
    err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        decode_json_and_save(local_response_buffer);
        ESP_LOGI(TAG_WIFI, "Recieved data from database");
    } else {
        ESP_LOGE(TAG_WIFI, "HTTP POST request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);

    return err;
}

#include "esp_http_server.h"

esp_err_t serve_html(httpd_req_t *req) {

    esp_err_t err = ESP_OK;
    if (err != ESP_OK) {
        ESP_LOGE("HTTP", "Data");
        httpd_resp_send_404(req); // Zwróć błąd 404, jeśli odczyt nie powiedzie się
        return ESP_FAIL;
    }
    
    const char* html_content = 
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<title>ESP Config</title>"
        "<style>"
        "body {"
        "    font-family: 'Arial', sans-serif;"
        "    text-align: center;"
        "    margin: 20px;"
        "    background: linear-gradient(to bottom, #f0f8ff, #e6e6fa);"
        "    color: #333;"
        "}"
        "h1 {"
        "    font-size: 36px;"
        "    color: #1e4e8c;"
        "    text-shadow: 3px 3px 5px rgba(0, 0, 0, 0.3);"
        "    font-weight: bold;"
        "    margin-bottom: 30px;"
        "}"
        "h2 {"
        "    font-size: 22px;"
        "    color: #1e4e8c;"
        "    margin-bottom: 10px;"
        "    text-shadow: 1px 1px 1px #ddd;"
        "}"
        "form {"
        "    margin: 20px auto;"
        "    width: 600px;"
        "    text-align: left;"
        "    background: #ffffff;"
        "    padding: 20px;"
        "    border-radius: 10px;"
        "    box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);"
        "}"
        "input {"
        "    margin-bottom: 10px;"
        "    width: calc(100% - 16px);"
        "    padding: 8px;"
        "    border: 1px solid #ccc;"
        "    border-radius: 5px;"
        "    box-sizing: border-box;"
        "    font-size: 14px;"
        "}"
        "button {"
        "    padding: 12px 25px;"
        "    background-color: #1e4e8c;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 5px;"
        "    cursor: pointer;"
        "    font-size: 16px;"
        "    transition: background-color 0.3s ease, transform 0.2s ease;"
        "    margin-top: 30px;"
        "    width: 100%;"
        "}"
        "button:hover {"
        "    background-color: #165d8d;"
        "    transform: scale(1.05);"
        "}"
        ".device-section {"
        "    display: grid;"
        "    grid-template-columns: repeat(2, 1fr);"
        "    gap: 20px;"
        "}"
        ".device-group {"
        "    border: 1px solid #ddd;"
        "    padding: 15px;"
        "    border-radius: 10px;"
        "    background: #f9f9f9;"
        "    box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);"
        "}"
        ".device-group h3 {"
        "    margin: 0 0 10px 0;"
        "    font-size: 18px;"
        "    color: #1e4e8c;"
        "    text-align: center;"
        "}"
        "footer {"
        "    margin-top: 20px;"
        "    font-size: 12px;"
        "    color: #666;"
        "}"
        "</style>"
        "</head>"
        "<body>"
        "    <h1>Master Device Configuration</h1>"
        "    <form action='/submit' method='POST'>"
        "        <h2>WiFi Settings</h2>"
        "        SSID:<br>"
        "        <input type='text' name='ssid' required><br>"
        "        Password:<br>"
        "        <input type='password' name='password' required><br>"
        "        <h2>Devices</h2>"
        "        <div class='device-section'>"
        "            <div class='device-group'>"
        "                <h3>Device 1</h3>"
        "                Serial Number:<br>"
        "                <input type='text' name='serial1'><br>"
        "                MAC Address:<br>"
        "                <input type='text' name='mac1'><br>"
        "            </div>"
        "            <div class='device-group'>"
        "                <h3>Device 2</h3>"
        "                Serial Number:<br>"
        "                <input type='text' name='serial2'><br>"
        "                MAC Address:<br>"
        "                <input type='text' name='mac2'><br>"
        "            </div>"
        "            <div class='device-group'>"
        "                <h3>Device 3</h3>"
        "                Serial Number:<br>"
        "                <input type='text' name='serial3'><br>"
        "                MAC Address:<br>"
        "                <input type='text' name='mac3'><br>"
        "            </div>"
        "            <div class='device-group'>"
        "                <h3>Device 4</h3>"
        "                Serial Number:<br>"
        "                <input type='text' name='serial4'><br>"
        "                MAC Address:<br>"
        "                <input type='text' name='mac4'><br>"
        "            </div>"
        "            <div class='device-group'>"
        "                <h3>Device 5</h3>"
        "                Serial Number:<br>"
        "                <input type='text' name='serial5'><br>"
        "                MAC Address:<br>"
        "                <input type='text' name='mac5'><br>"
        "            </div>"
        "            <div class='device-group'>"
        "                <h3>Device 6</h3>"
        "                Serial Number:<br>"
        "                <input type='text' name='serial6'><br>"
        "                MAC Address:<br>"
        "                <input type='text' name='mac6'><br>"
        "            </div>"
        "            <div class='device-group'>"
        "                <h3>Device 7</h3>"
        "                Serial Number:<br>"
        "                <input type='text' name='serial7'><br>"
        "                MAC Address:<br>"
        "                <input type='text' name='mac7'><br>"
        "            </div>"
        "            <div class='device-group'>"
        "                <h3>Device 8</h3>"
        "                Serial Number:<br>"
        "                <input type='text' name='serial8'><br>"
        "                MAC Address:<br>"
        "                <input type='text' name='mac8'><br>"
        "            </div>"
        "            <div class='device-group'>"
        "                <h3>Device 9</h3>"
        "                Serial Number:<br>"
        "                <input type='text' name='serial9'><br>"
        "                MAC Address:<br>"
        "                <input type='text' name='mac9'><br>"
        "            </div>"
        "            <div class='device-group'>"
        "                <h3>Device 10</h3>"
        "                Serial Number:<br>"
        "                <input type='text' name='serial10'><br>"
        "                MAC Address:<br>"
        "                <input type='text' name='mac10'><br>"
        "            </div>"
        "        </div>"
        "        <button type='submit'>Submit</button>"
        "    </form>"
        "    <footer>"
        "        &copy; 2024 ESP Configurator. All rights reserved."
        "    </footer>"
        "</body>"
        "</html>";
    
    httpd_resp_send(req, html_content, HTTPD_RESP_USE_STRLEN);
    return err;
}

static httpd_uri_t html_uri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = serve_html,
    .user_ctx = NULL
};

// static httpd_uri_t form_uri = {
//     .uri = "/submit",
//     .method = HTTP_POST,
//     .handler = handle_form_submission,
//     .user_ctx = NULL
// };

void start_webserver(void) {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    ESP_ERROR_CHECK(httpd_start(&server, &config));

    // Rejestracja handlerów
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &html_uri));
    //ESP_ERROR_CHECK(httpd_register_uri_handler(server, &form_uri));
}

