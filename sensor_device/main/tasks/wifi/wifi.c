#include "wifi.h"
#include "esp_err.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"
#include "freertos/idf_additions.h"
#include "tasks/data/data.h"
#include "tasks/tasks.h"
#include <stdint.h>
#include <string.h>


static const char *TAG_WIFI = "WIFI";
static wifi_flags_t flags = {0};
static esp_netif_t *esp_netif_sta;
static EventGroupHandle_t esp_now_evt_group;
static esp_now_peer_info_t peer;


wifi_flags_t* get_wifi_flags(void)
{
    return &flags;
}


static esp_netif_t *wifi_init_sta(void)
{
    esp_netif_t *esp_netif_sta = esp_netif_create_default_wifi_sta();
    ESP_LOGI(TAG_WIFI, "Starting station...");

    return esp_netif_sta;
}


esp_err_t wifi_init()
{
    esp_err_t err = ESP_OK;

    if (!flags.wifi_initialized)
    {
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
        ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
        ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA));

        esp_netif_sta = wifi_init_sta();


        ESP_ERROR_CHECK(esp_wifi_start());

        flags.wifi_initialized = 1;
        esp_netif_set_default_netif(esp_netif_sta);
    }
    return err;
}


static void esp_now_recv_cb(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len)
{

    static recv_data_t recieve_data;
    ESP_LOGI(TAG_WIFI, "%d bytes incoming from " MACSTR, len, MAC2STR(recv_info->src_addr));

    // CHECK DATA LENGTH
    if(len != sizeof(recieve_data))
    {
        ESP_LOGE(TAG_WIFI, "Unexpected data length: %d != %u", len, (int) sizeof(recv_data_t));
        return;
    }

    memcpy(&recieve_data, data, len);

    // CHECK IF DATA WAS WELL ADDRESSED
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
        // SNIFFER TASK OFF
        sniffer_suspend();

        peer.ifidx = WIFI_IF_STA;
        peer.channel = recieve_data.channel;
        memcpy(peer.peer_addr, recv_info->src_addr, sizeof(peer.peer_addr)); 
        esp_now_add_peer(&peer);
        ESP_LOGI(TAG_WIFI, "New peer added, channel = %d", recieve_data.channel);
        ESP_ERROR_CHECK(esp_wifi_set_channel(recieve_data.channel, WIFI_SECOND_CHAN_NONE));
        ESP_LOGI(TAG_WIFI, "Channel set = %d", recieve_data.channel);
    }
    else if (peer_num.total_num != 0 && memcmp(recv_info->src_addr, peer.peer_addr, sizeof(peer.peer_addr)))
    {
        ESP_LOGW(TAG_WIFI, "Peer's MAC address is not valid");
        return;
    }

    //RESET TIMER
    reset_sniffer_timer();
    
    // SEND DATA TO THE QUEUE 
    if  (xQueueSend(get_queue_handle(), &recieve_data, 0) != pdTRUE) {
        ESP_LOGW(TAG_WIFI, "Queue full, discarded");
    }

    return;
}

void delete_my_peer(void)
{
    esp_now_del_peer(peer.peer_addr);
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
        // ESP_ERROR_CHECK(esp_wifi_set_channel(6, WIFI_SECOND_CHAN_NONE));
        ESP_LOGI(TAG_WIFI, "ESP_NOW INITIALIZATION...");
           
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

    int number_of_retries = 0;

    while (number_of_retries < MAX_SEND_RETRIES)
    {
        // Peer validation
        esp_now_peer_num_t peer_num;
        esp_now_get_peer_num(&peer_num);
        if (peer_num.total_num == 0)
        {
            ESP_LOGI(TAG_WIFI, "Error, peer does not exists" MACSTR, MAC2STR(data.mac_address));
            err = ESP_NOW_SEND_FAIL;
            return err;
        }

        // Send it
        ESP_LOGI(TAG_WIFI, "Sending data to " MACSTR, MAC2STR(peer.peer_addr));
        err = esp_now_send(peer.peer_addr, (uint8_t*)&data, sizeof(data));

        if(err != ESP_OK)
        {
            ESP_LOGE(TAG_WIFI, "Send error (%d)", err);
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
            ESP_LOGI(TAG_WIFI, "Data sent!");
            return err;
        }
        number_of_retries++;
         
    }

    if (err != ESP_OK)
        ESP_LOGE(TAG_WIFI, "ESP-NOW send error (%s)", esp_err_to_name(err));
            
    ESP_LOGI(TAG_WIFI, "Number of retries = %d", number_of_retries);
    return ESP_OK;
}

