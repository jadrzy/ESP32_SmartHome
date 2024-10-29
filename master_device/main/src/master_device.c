#include "master_device.h"
#include "components/components.h"
#include "esp_err.h"
#include "esp_log.h"
#include "components/nvs/nvs.h"
#include "components/wifi/wifi.h"
#include <tasks/task.h>
#include "freertos/idf_additions.h"
#include "nvs_flash.h"
#include <stdio.h>
#include <string.h>

void app_main(void)
{
    ESP_ERROR_CHECK(memory_setup());
    ESP_ERROR_CHECK(wifi_init());
    ESP_ERROR_CHECK(my_esp_now_init());


    while(1)
    {
        vTaskDelay(10000 / portTICK_PERIOD_MS);
        wifi_reboot();
    }

    vTaskSuspend(NULL);
}

