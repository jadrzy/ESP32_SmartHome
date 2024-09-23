#include "device.h"
#include "freertos/idf_additions.h"

void app_main(void)
{
    ESP_LOGI("TAG", "app_started");
    vTaskDelay(100 / portTICK_PERIOD_MS);
    initialize_sensors();    
    vTaskDelay(100 / portTICK_PERIOD_MS);
    initialize_tasks();
    vTaskSuspend(NULL);
}
