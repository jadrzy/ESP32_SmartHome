#include "wifi.h"
#include "components/nvs/nvs.h"
#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"
#include <stdio.h>
#include <string.h>

// ADDED ////////////////////////////////////
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

/*DHCP server option*/
#define DHCPS_OFFER_DNS             0x02

static EventGroupHandle_t s_wifi_event_group;

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *) event_data;
        ESP_LOGI(TAG_AP, "Station "MACSTR" joined, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *) event_data;
        ESP_LOGI(TAG_AP, "Station "MACSTR" left, AID=%d, reason:%d",
                 MAC2STR(event->mac), event->aid, event->reason);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
        ESP_LOGI(TAG_STA, "Station started");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(TAG_STA, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}
///////////////////////////////////////////

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

// ADDED ////////////////////////////////////////////////////////////////////////////////

static void softap_set_dns_addr(esp_netif_t *esp_netif_ap,esp_netif_t *esp_netif_sta)
{
    esp_netif_dns_info_t dns;
    esp_netif_get_dns_info(esp_netif_sta,ESP_NETIF_DNS_MAIN,&dns);
    uint8_t dhcps_offer_option = DHCPS_OFFER_DNS;
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_dhcps_stop(esp_netif_ap));
    ESP_ERROR_CHECK(esp_netif_dhcps_option(esp_netif_ap, ESP_NETIF_OP_SET, ESP_NETIF_DOMAIN_NAME_SERVER, &dhcps_offer_option, sizeof(dhcps_offer_option)));
    ESP_ERROR_CHECK(esp_netif_set_dns_info(esp_netif_ap, ESP_NETIF_DNS_MAIN, &dns));
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_dhcps_start(esp_netif_ap));
}

/////////////////////////////////////////////////////////////////////////////////////////////

esp_err_t wifi_init(bool setup_active)
{
    esp_err_t err = ESP_OK;


    // ADDED //////////////////////////////////////////////////////
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

    ////////////////////////////////////////////////////////////////////
    
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



    // ADDED //////////////////////////////////////////
    /*
     * Wait until either the connection is established (WIFI_CONNECTED_BIT) or
     * connection failed for the maximum number of re-tries (WIFI_FAIL_BIT).
     * The bits are set by event_handler() (see above)
     */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned,
     * hence we can test which event actually happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG_STA, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_STA_SSID, EXAMPLE_ESP_WIFI_STA_PASSWD);
        softap_set_dns_addr(esp_netif_ap,esp_netif_sta);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG_STA, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_STA_SSID, EXAMPLE_ESP_WIFI_STA_PASSWD);
    } else {
        ESP_LOGE(TAG_STA, "UNEXPECTED EVENT");
        return;
    }

    /* Set sta as the default interface */
    esp_netif_set_default_netif(esp_netif_sta);

    /* Enable napt on the AP netif */
    if (esp_netif_napt_enable(esp_netif_ap) != ESP_OK) {
        ESP_LOGE(TAG_STA, "NAPT not enabled on the netif: %p", esp_netif_ap);
    }


    /////////////////////////////////////////////////////////////////////////////////////




    
    return err;
}

