#include "device.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"
#include "freertos/idf_additions.h"
#include "tasks/data/data.h"
#include "tasks/wifi/wifi.h"
#include <stdint.h>


void app_main(void)
{
    ESP_LOGI("TAG", "app_started");
    ESP_ERROR_CHECK(memory_setup());
    debug();

    wifi_init();
    my_esp_now_init();
    
    vTaskDelay(100 / portTICK_PERIOD_MS);
    initialize_sensors();    
    vTaskDelay(500 / portTICK_PERIOD_MS);
    
    initialize_tasks();
    
    while(1){
        uint8_t primary = 0;
        wifi_second_chan_t secondary;
        esp_wifi_get_channel(&primary, &secondary);
        ESP_LOGI("TAG", "ESP-NOW channel = %d", primary);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    vTaskSuspend(NULL);
}
