#include "wifi.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"
#include "tasks/data/data.h"
#include "tasks/tasks.h"
#include <string.h>


static const char *TAG_WIFI = "WIFI";
static wifi_flags_t flags = {0};
static esp_netif_t *esp_netif_ap;


static wifi_config_t wifi_ap_config = {
    .ap = {
        .ssid = "network test",
        .password = "0123456789",
        .channel = 6,
        .max_connection = 0,
        .ssid_hidden = true,
        .authmode = WIFI_AUTH_WPA3_PSK,
        .pmf_cfg = {
            .required = true,
        }

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
    // STA STARTED 
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
    {
        ESP_LOGI(TAG_WIFI, "Station started");
        esp_wifi_connect();
    } 
}


static esp_netif_t *wifi_init_ap(void)
{
    esp_netif_t *esp_netif_ap = esp_netif_create_default_wifi_ap();
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_config));
    ESP_LOGI(TAG_WIFI, "Starting AP...");

    return esp_netif_ap;
}

// static esp_netif_t *wifi_init_sta(void)
// {
//     esp_netif_t *esp_netif_sta = esp_netif_create_default_wifi_sta();
//     ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_sta_config));
//     ESP_LOGI(TAG_WIFI, "Starting station...");
//
//     return esp_netif_sta;
// }


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
        ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_AP));

        esp_netif_ap = wifi_init_ap();


        ESP_ERROR_CHECK(esp_wifi_start());

        flags.wifi_initialized = 1;
        esp_netif_set_default_netif(esp_netif_ap);
    }
    return err;
}



static EventGroupHandle_t esp_now_evt_group;
static esp_now_peer_info_t peer;

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

    slave_device_t slave;
    get_slave_device(slave.serial_number, slave.mac_address);
    if (!strcmp(recieve_data.serial, slave.serial_number))
    {
        ESP_LOGI(TAG_WIFI, "ESP-NOW recieve data error. Serial does not match");
        return;
    }

    esp_now_peer_num_t peer_num;
    esp_now_get_peer_num(&peer_num);
    if (peer_num.total_num == 0)
    {
        peer.ifidx = WIFI_IF_AP;
        peer.channel = 6;
        memcpy(peer.peer_addr, recieve_data.mac_address, sizeof(recieve_data.mac_address)); 
        esp_now_add_peer(&peer);
    }
    else if (peer_num.total_num != 0 && memcmp(recieve_data.mac_address, peer.peer_addr, sizeof(peer.peer_addr)))
    {
        ESP_LOGI(TAG_WIFI, "Recieve MAC = %2x:%2x:%2x:%2x:%2x:%2x", 
                 recieve_data.mac_address[0],
                 recieve_data.mac_address[1],
                 recieve_data.mac_address[2],
                 recieve_data.mac_address[3],
                 recieve_data.mac_address[4],
                 recieve_data.mac_address[5]);

        ESP_LOGI(TAG_WIFI, "Recieve MAC = %2x:%2x:%2x:%2x:%2x:%2x", 
                 peer.peer_addr[0],
                 peer.peer_addr[1],
                 peer.peer_addr[2],
                 peer.peer_addr[3],
                 peer.peer_addr[4],
                 peer.peer_addr[5]);

        ESP_LOGW(TAG_WIFI, "Peer's MAC address is not valid");
        return;
    }

    // activate response 
    if (xQueueSend(get_queue_handle(), &recieve_data, 0) != pdTRUE) {
        ESP_LOGW(TAG_WIFI, "Queue full, discarded");
    }
    return;
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
        ESP_ERROR_CHECK(esp_wifi_set_protocol(ESP_IF_WIFI_AP, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR));

        ESP_ERROR_CHECK(esp_now_init());
        ESP_LOGI(TAG_WIFI, "ESP_NOW INITIALIZATION...");
           

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
    err = esp_now_send(data.mac_address, (uint8_t*)&data, sizeof(data));

    if(err != ESP_OK)
    {
        ESP_LOGE(TAG_WIFI, "Send error1 (%d)", err);
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
            ESP_LOGE(TAG_WIFI, "ESP-NOW send error (%d)", err);
            return err;
        }
        err = ESP_ERR_TIMEOUT;
        ESP_LOGE(TAG_WIFI, "ESP-NOW send error (%d)", err);
        return err;
    }

    ESP_LOGI(TAG_WIFI, "Sent!");
    return ESP_OK;
}

