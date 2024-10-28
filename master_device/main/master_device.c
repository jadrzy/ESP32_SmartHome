#include "master_device.h"
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

        vTaskDelay(10000 / portTICK_PERIOD_MS);
    while (1)
    {

        wifi_set_ap_setup(true);
        wifi_update();
        ESP_LOGI("TAG", "SET UP MODE ON");
        vTaskDelay(60000 / portTICK_PERIOD_MS);

        wifi_set_ap_setup(false);
        wifi_update();
        ESP_LOGI("TAG", "SET UP MODE OFF");
        vTaskDelay(60000 / portTICK_PERIOD_MS);
    }
}
