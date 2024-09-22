#include "device.h"

void app_main(void)
{
    ESP_LOGI("TAG", "app_started");
    initialize_sensors();    
    initialize_tasks();
    vTaskSuspend(NULL);
}
