#include "include/main.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"
#include "freertos/idf_additions.h"
#include "include/data.h"
#include "include/task.h"
#include "include/wifi.h"
#include <stdint.h>
#include <string.h>
#include "include/led.h"

#include "include/ntp.h"

static const char TAG_MAIN[] = "MAIN";

void app_main(void)
{
    ESP_LOGI(TAG_MAIN, "APP STARTED");
    ESP_ERROR_CHECK(memory_setup());
    ESP_ERROR_CHECK(init_led());
    ESP_ERROR_CHECK(recv_queue_task_init());
    ESP_ERROR_CHECK(wifi_init());
    ESP_ERROR_CHECK(my_esp_now_init());


    vTaskSuspend(NULL);
}

