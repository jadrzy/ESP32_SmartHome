#include "wifi.h"
#include "components/nvs/nvs.h"
#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"
#include <stdio.h>
#include <string.h>

static const char *TAG_WIFI = "WIFI";


static esp_netif_t *wifi_init_ap(bool visibility)
{
    wifi_config_t wifi_ap_config = {
        .ap = {
            .channel = 1,
            .max_connection = 1,
            .pmf_cfg = {
                .required = false,
            },
        },
    };
    
    char * ssid_ap = get_master_device_cred().serial;
    char * psswd_ap;
    sprintf(psswd_ap, "%" PRIu64, get_master_device_cred().mac);
    strcpy((char *) wifi_ap_config.ap.ssid, ssid_ap);    
    strcpy((char *) wifi_ap_config.ap.password, psswd_ap);    
    wifi_ap_config.ap.ssid[31] = '\0';
    wifi_ap_config.ap.password[63] = '\0';
    wifi_ap_config.ap.ssid_len = strlen(ssid_ap);

    if (visibility){
        wifi_ap_config.ap.ssid_hidden = false;
        wifi_ap_config.ap.max_connection = 1;
    }else{
        wifi_ap_config.ap.ssid_hidden = true;
        wifi_ap_config.ap.max_connection = 0;
    }

    esp_netif_t *esp_netif_ap = esp_netif_create_default_wifi_ap();
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_config));
    ESP_LOGI(TAG_WIFI, "WIFI SOFT_AP SETUP...");

    return esp_netif_ap;
}

static esp_netif_t *wifi_init_sta(void)
{
    static wifi_config_t wifi_sta_config = {
        .sta = {
            .scan_method = WIFI_ALL_CHANNEL_SCAN,
            .failure_retry_cnt = 10,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };

    strcpy((char *) wifi_sta_config.sta.ssid, get_station_cred().ssid);
    strcpy((char *) wifi_sta_config.sta.password, get_station_cred().psswd);
    // For safety (casting char *)
    wifi_sta_config.sta.ssid[31] = '\0';
    wifi_sta_config.sta.password[63] = '\0';

    esp_netif_t *esp_netif_sta = esp_netif_create_default_wifi_sta();
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_sta_config));
    ESP_LOGI(TAG_WIFI, "WIFI SM SETUP...");

    return esp_netif_sta;
}


esp_err_t wifi_init(bool setup_active)
{
    esp_err_t err = ESP_OK;

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_APSTA));

    esp_netif_t *esp_netif_ap = wifi_init_ap(setup_active);
    ESP_LOGI(TAG_WIFI, "WIFI AP INITIALIZATION...");

    esp_netif_t *esp_netif_sta = wifi_init_sta();
    ESP_LOGI(TAG_WIFI, "WIFI SM INITIALIZATION...");

    err = esp_wifi_start();
    if (err != ESP_OK)
        ESP_LOGE(TAG_WIFI, "ERROR STARTING WIFI...");

    return err;
}

