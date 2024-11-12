#include "device.h"
#include "freertos/idf_additions.h"
#include "tasks/data/data.h"
#include "tasks/wifi/wifi.h"


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
    
    vTaskSuspend(NULL);
}
