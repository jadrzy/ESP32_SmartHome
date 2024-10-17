#include "wifi.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"
#include <string.h>



static const char *TAG = "WIFI";
static wifi_config_t wifi_ap_config = {
    .ap = {
        .ssid = "SETUP",
        .ssid_len = strlen("SETUP"),
        .channel = 3,
        .password = "PASS",
        .max_connection = 1,
        .pmf_cfg = {
            .required = false,
        },
    },
};

static wifi_config_t wifi_sta_config = {
    .sta = {
        .ssid = "LE",
        .password = "PASS",
        .scan_method = WIFI_ALL_CHANNEL_SCAN,
        .failure_retry_cnt = 10,
        .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
    },
};

static esp_netif_t *wifi_init_ap(void)
{
    esp_netif_t *esp_netif_ap = esp_netif_create_default_wifi_ap();
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_config));
    ESP_LOGI(TAG, "wifi init soft ap done");

    return esp_netif_ap;
}

static esp_netif_t *wifi_init_sta(void)
{
    esp_netif_t *esp_netif_sta = esp_netif_create_default_wifi_sta();
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_sta_config));
    ESP_LOGI(TAG, "wifi init sta done");

    return esp_netif_sta;
}
void wifi_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_LOGI(TAG, "ESP_AP");
    esp_netif_t *esp_netif_ap = wifi_init_ap();

    ESP_LOGI(TAG, "ESP_STA");
    esp_netif_t *esp_netif_sta = wifi_init_sta();
    ESP_ERROR_CHECK(esp_wifi_start());

}



void settings_AP(void);

/* WiFi should start before using ESPNOW */
static void example_wifi_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(ESPNOW_WIFI_MODE) );
    ESP_ERROR_CHECK( esp_wifi_start());
    ESP_ERROR_CHECK( esp_wifi_set_channel(CONFIG_ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE));

}

