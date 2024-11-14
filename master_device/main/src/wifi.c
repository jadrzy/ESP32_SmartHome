#include "include/wifi.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_now.h"
#include "esp_wifi_types_generic.h"
#include "freertos/idf_additions.h"
#include "include/components.h"
#include "include/data.h"
#include "include/nvs.h"
#include "include/task.h"
#include <stdint.h>
#include <string.h>


static const char *TAG_WIFI = "WIFI";

static esp_now_peer_info_t peer_list[NUMBER_OF_DEVICES];

static wifi_flags_t flags = {0};

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
        .channel = 1,
        .max_connection = 1,
        .ssid_hidden = true,
        .authmode = WIFI_AUTH_WPA3_EXT_PSK,
        .pmf_cfg = {
            .required = true,
        },
    },
};


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
        esp_wifi_connect();
    } 

    // STA GOT IP
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(TAG_WIFI, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
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
        wifi_ap_config.ap.max_connection = 1;
    else
        wifi_ap_config.ap.max_connection = 0;

    esp_netif_t *esp_netif_ap = esp_netif_create_default_wifi_ap();
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_config));
    ESP_LOGI(TAG_WIFI, "WIFI SOFT_AP STARTED, SSID:%s password:%s",ssid_ap, psswd_ap);

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
    ESP_LOGI(TAG_WIFI, "WIFI SM STARTED...");

    return esp_netif_sta;
}


void wifi_reboot(void)
{
    if(flags.esp_now_initiated)
    {
        esp_now_deinit();
        flags.esp_now_initiated = 0;
    }

    if (flags.wifi_initialized)
    {
        ESP_ERROR_CHECK(esp_wifi_stop());
        ESP_ERROR_CHECK(esp_wifi_deinit());
        ESP_ERROR_CHECK(esp_event_loop_delete_default());
        esp_netif_destroy(esp_netif_sta);
        esp_netif_destroy(esp_netif_ap);
        flags.wifi_initialized = 0;

    }
    ESP_LOGI(TAG_WIFI, "WIFI uninitialized...");

    ESP_ERROR_CHECK(wifi_init());
    ESP_ERROR_CHECK(my_esp_now_init());
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

        ESP_ERROR_CHECK(esp_wifi_start());

        flags.wifi_initialized = 1;
        esp_netif_set_default_netif(esp_netif_sta);
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
            peer_list[peers_count].ifidx = WIFI_IF_AP;
            peer_list[peers_count].channel = 6;
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
        ESP_ERROR_CHECK(esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR));

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

static int number_of_retries;

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
    ESP_LOGI(TAG_WIFI, "Sending data request to " MACSTR, MAC2STR(data.mac_address));

    number_of_retries = 0;

    while ( number_of_retries < 100)
    {
        uint8_t primary = 0;
        wifi_second_chan_t secondary = 0;
        esp_wifi_get_channel(&primary, &secondary);
        ESP_LOGI(TAG_WIFI, "ESP-NOW channel = %d", primary);
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
            ESP_LOGI(TAG_WIFI, "Sent!");
            return err;
        }
        ++number_of_retries; 
    }

    if (err != ESP_OK)    
        ESP_LOGE(TAG_WIFI, "ESP-NOW send error (%s)", esp_err_to_name(err));

    ESP_LOGI(TAG_WIFI, "Number of retries = %d", number_of_retries);
    return ESP_OK;
}

