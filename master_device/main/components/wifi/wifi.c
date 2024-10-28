#include "wifi.h"
#include "components/nvs/nvs.h"
#include "esp_err.h"
#include "esp_event_base.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif_net_stack.h"
#include "esp_netif.h"
#include "lwip/def.h"
#include "nvs_flash.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "lwip/lwip_napt.h"
#include "lwip/err.h"
#include "lwip/sys.h"


static const char *TAG_WIFI = "WIFI";

static bool wifi_setup_mode = false;

static esp_netif_t *esp_netif_ap;
static esp_netif_t *esp_netif_sta;


static EventGroupHandle_t s_wifi_event_group;

static wifi_config_t wifi_sta_config = {
    .sta = {
        .scan_method = WIFI_ALL_CHANNEL_SCAN,
        .failure_retry_cnt = 10,
        .channel = 0,
        .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
    },
};


wifi_config_t wifi_ap_config = {
    .ap = {
        .channel = 0,
        .max_connection = 1,
        .ssid_hidden = true,
        .authmode = WIFI_AUTH_WPA3_EXT_PSK,
        .pmf_cfg = {
            .required = true,
        },
    },
};

static void mac_64_str(uint64_t u64, char *str)
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
    // AP EVENTS
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *) event_data;
        if (wifi_setup_mode == false)
        {
            ESP_ERROR_CHECK(esp_wifi_deauth_sta(0));
            ESP_LOGI(TAG_WIFI, "Setup mode is disabled");
        } else {
            ESP_LOGI(TAG_WIFI, "Station "MACSTR" joined, AID=%d",
                     MAC2STR(event->mac), event->aid);
        }
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *) event_data;
        ESP_LOGI(TAG_WIFI, "Station "MACSTR" left, AID=%d, reason:%d",
                 MAC2STR(event->mac), event->aid, event->reason);

    // STATION EVENTS
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
        ESP_LOGI(TAG_WIFI, "Station started");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        ESP_LOGI(TAG_WIFI, "Station connected, SSID=%s ...", get_station_cred().ssid);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG_WIFI, "Station disconnected, SSID=%s ...", get_station_cred().ssid);
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(TAG_WIFI, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
    }
}


static esp_netif_t *wifi_init_ap(void)
{

    char ssid_ap[33]; 
    strcpy(ssid_ap, get_master_device_cred().serial);
    char psswd_ap[65];
    uint64_t mac_address = get_master_device_cred().mac;
    mac_64_str(mac_address, psswd_ap);

    strcpy((char *) wifi_ap_config.ap.ssid, ssid_ap);    
    strcpy((char *) wifi_ap_config.ap.password, psswd_ap);    

    wifi_ap_config.ap.ssid[31] = '\0';
    wifi_ap_config.ap.password[63] = '\0';


    esp_netif_t *esp_netif_ap = esp_netif_create_default_wifi_ap();
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_config));
    ESP_LOGI(TAG_WIFI, "WIFI SOFT_AP STARTED, SSID:%s password:%s",ssid_ap, psswd_ap);

    return esp_netif_ap;
}

static void set_ap_setup()
{
    if (wifi_setup_mode)
    {
        wifi_ap_config.ap.max_connection = 1;
    } else {
        wifi_ap_config.ap.max_connection = 0;
    }
}

static void set_sm_cred(void)
{
    strcpy((char *) wifi_sta_config.sta.ssid, get_station_cred().ssid);
    strcpy((char *) wifi_sta_config.sta.password, get_station_cred().psswd);
    // For safety (casting char *)
    wifi_sta_config.sta.ssid[31] = '\0';
    wifi_sta_config.sta.password[63] = '\0';
}

static esp_netif_t *wifi_init_sta(void)
{
    set_sm_cred();

    esp_netif_t *esp_netif_sta = esp_netif_create_default_wifi_sta();
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_sta_config));
    ESP_LOGI(TAG_WIFI, "WIFI SM STARTED...");

    return esp_netif_sta;
}



void wifi_set_ap_setup(bool state)
{
    wifi_setup_mode = state;
}

void wifi_update(void)
{
    ESP_ERROR_CHECK(esp_wifi_stop());

    set_ap_setup();
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_config));

    set_sm_cred();
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_sta_config));

    ESP_ERROR_CHECK(esp_wifi_start());

    esp_netif_set_default_netif(esp_netif_sta);

    ESP_ERROR_CHECK(esp_wifi_set_protocol(ESP_IF_WIFI_AP, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR));
}


esp_err_t wifi_init()
{
    esp_err_t err = ESP_OK;

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    s_wifi_event_group = xEventGroupCreate();

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

    ESP_ERROR_CHECK(esp_wifi_start());

    esp_netif_set_default_netif(esp_netif_sta);

    ESP_ERROR_CHECK(esp_wifi_set_protocol(ESP_IF_WIFI_AP, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR));

    return err;
}


// ESP_NOW
