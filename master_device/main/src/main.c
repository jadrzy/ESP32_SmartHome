#include "include/main.h"
#include "esp_err.h"
#include "esp_log.h"
#include "include/data.h"
#include "include/task.h"
#include "include/wifi.h"
#include <string.h>

static const char TAG_MAIN[] = "MAIN";

void app_main(void)
{

    slave_device_t devices[NUMBER_OF_DEVICES];

    send_data_t data ;

    ESP_LOGI(TAG_MAIN, "APP STARTED");
    ESP_ERROR_CHECK(memory_setup());
    ESP_ERROR_CHECK(recv_queue_task_init());

    get_slave_devices(devices);
    strcpy(data.serial, devices[0].serial_number);
    memcpy(data.mac_address, devices[0].mac_address, sizeof(data.mac_address));
    data.auto_light = devices[0].light_control.auto_light;
    data.light_value = devices[0].light_control.light_value;

    ESP_LOGI(TAG_MAIN, "Serial = %s,\tMac = %2x:%2x:%2x:%2x:%2x:%2x", 
             data.serial, 
             data.mac_address[0], 
             data.mac_address[1],
             data.mac_address[2],
             data.mac_address[3],
             data.mac_address[4],
             data.mac_address[5]);

    ESP_ERROR_CHECK(wifi_init());
    ESP_ERROR_CHECK(my_esp_now_init());


     while(1)
     {
         vTaskDelay(1000 / portTICK_PERIOD_MS);
         send_espnow_data(data);
    //     wifi_reboot();
     }

    vTaskSuspend(NULL);
}

