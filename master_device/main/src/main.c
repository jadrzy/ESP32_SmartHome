#include "main.h"

static const char TAG_MAIN[] = "MAIN";

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

