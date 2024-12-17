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

#include "include/led.h"
#include "include/ntp.h"
#include "portmacro.h"

#define MAX_RETRIES_ESP_NOW 10

static const char *TAG_WIFI = "WIFI";

static esp_now_peer_info_t peer_list[NUMBER_OF_DEVICES];

static wifi_flags_t flags = {0};

static TimerHandle_t setup_timer = NULL;

static esp_netif_t *esp_netif_ap;
static esp_netif_t *esp_netif_sta;


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
        if (flags.reboot == 0)
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

    if (flags.setup_mode == 1)
    {
        wifi_ap_config.ap.max_connection = 1;
        ESP_LOGI(TAG_WIFI, "WIFI SETUP MODE = ON");
        ESP_LOGI(TAG_WIFI, "SSID = %s", wifi_ap_config.ap.ssid);
        ESP_LOGI(TAG_WIFI, "PSSWD = %s", wifi_ap_config.ap.password);

    }
    else
    {
        wifi_ap_config.ap.max_connection = 0;
        ESP_LOGI(TAG_WIFI, "WIFI SETUP MODE = OFF");
    }

    esp_netif_t *esp_netif_ap = esp_netif_create_default_wifi_ap();
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_config));

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


void wifi_reboot(void)
{
    ESP_LOGI(TAG_WIFI, "REBOOT...");
    esp_wifi_stop();

    if (flags.esp_now_initiated == 1)
    {
        esp_now_deinit(); 
        flags.esp_now_initiated = 0;
    }

    if (flags.wifi_initialized == 1)
    {
        ESP_ERROR_CHECK(esp_wifi_deinit());
        esp_netif_deinit();
        esp_netif_destroy(esp_netif_sta);
        esp_netif_destroy(esp_netif_ap);
        ESP_ERROR_CHECK(esp_event_loop_delete_default());
        flags.wifi_initialized = 0;
    }

    wifi_init();
    my_esp_now_init();
    esp_wifi_start();
}


static esp_err_t set_setup_mode(bool state)
{
    esp_err_t err = ESP_OK;

    if (flags.setup_mode != state)
    {
        flags.setup_mode = state;
        flags.reboot = 1;
    }

    return err;
}

esp_err_t start_setup_mode(void)
{
    esp_err_t err = ESP_OK;
    set_setup_mode(true);
    if (!xTimerIsTimerActive(setup_timer))
    {
        xTimerReset(setup_timer, 5);
    }
    else 
        xTimerStart(setup_timer, 5);
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
    esp_restart();
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

        esp_netif_ap = wifi_init_ap();
        ESP_LOGI(TAG_WIFI, "WIFI AP INITIALIZATION...");

        esp_netif_sta = wifi_init_sta();
        ESP_LOGI(TAG_WIFI, "WIFI SM INITIALIZATION...");

        flags.wifi_initialized = 1;
        esp_netif_set_default_netif(esp_netif_sta);

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

    if (!flags.esp_now_initiated)
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




// HTTPS

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
            ESP_LOGI(TAG_WIFI, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG_WIFI, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG_WIFI, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG_WIFI, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            // Clean the buffer in case of a new request
            if (output_len == 0 && evt->user_data) {
                // we are just starting to copy the output data into the use
                memset(evt->user_data, 0, MAX_HTTP_OUTPUT_BUFFER);
            }
            /*
             *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
             *  However, event handler can also be used in case chunked encoding is used.
             */
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // If user_data buffer is configured, copy the response into the buffer
                int copy_len = 0;

                // if (evt->user_data) {
                //     // The last byte in evt->user_data is kept for the NULL character in case of out-of-bound access.
                //     copy_len = MIN(evt->data_len, (MAX_HTTP_OUTPUT_BUFFER - output_len));
                //     if (copy_len) {
                //         memcpy(evt->user_data + output_len, evt->data, copy_len);
                //     }
                // } else {
                //     int content_len = esp_http_client_get_content_length(evt->client);
                //     if (output_buffer == NULL) {
                //         // We initialize output_buffer with 0 because it is used by strlen() and similar functions therefore should be null terminated.
                //         output_buffer = (char *) calloc(content_len + 1, sizeof(char));
                //         output_len = 0;
                //         if (output_buffer == NULL) {
                //             ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
                //             return ESP_FAIL;
                //         }
                //     }
                //     copy_len = MIN(evt->data_len, (content_len - output_len));
                //     if (copy_len) {
                //         memcpy(output_buffer + output_len, evt->data, copy_len);
                //     }
                // }
                output_len += copy_len;
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG_WIFI, "HTTP_EVENT_ON_FINISH");
            if (output_buffer != NULL) {
                // Response is accumulated in output_buffer. Uncomment the below line to print the accumulated response
                // ESP_LOG_BUFFER_HEX(TAG, output_buffer, output_len);
                free(output_buffer);
                output_buffer = NULL;
            }
            output_len = 0;
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG_WIFI, "HTTP_EVENT_DISCONNECTED");
            // int mbedtls_err = 0;
            // esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
            // if (err != 0) {
            //     ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
            //     ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
            // }
            // if (output_buffer != NULL) {
            //     free(output_buffer);
            //     output_buffer = NULL;
            // }
            // output_len = 0;
            break;
        case HTTP_EVENT_REDIRECT:
            ESP_LOGI(TAG_WIFI, "HTTP_EVENT_REDIRECT");
            // esp_http_client_set_header(evt->client, "From", "user@example.com");
            // esp_http_client_set_header(evt->client, "Accept", "text/html");
            // esp_http_client_set_redirection(evt->client);
            break;
    }
    return ESP_OK;
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
    ESP_LOGI(TAG_WIFI, "FLAGA");

// POST
    esp_http_client_set_url(client, "http://192.168.0.210:5000/data");
    char * string_new = ""
    esp_http_client_set_post_field(client, string_JSON, strlen(string_JSON));
    esp_http_client_set_header(client, "Content-Type", "application/json");

    ESP_LOGI(TAG_WIFI, "%s", string_JSON);

    esp_http_client_set_method(client, HTTP_METHOD_POST);
    err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG_WIFI, "HTTP POST Status = %d, content_length = %"PRId64,
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG_WIFI, "HTTP POST request failed: %s", esp_err_to_name(err));
    }
    
    esp_http_client_cleanup(client);

    return err;
}
