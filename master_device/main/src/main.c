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

static const char TAG_MAIN[] = "MAIN";
slave_device_t devices[NUMBER_OF_DEVICES];

void app_main(void)
{
    ESP_LOGI(TAG_MAIN, "APP STARTED");
    ESP_ERROR_CHECK(memory_setup());
    ESP_ERROR_CHECK(recv_queue_task_init());
    ESP_ERROR_CHECK(wifi_init());
    ESP_ERROR_CHECK(my_esp_now_init());

    get_slave_devices(devices);
    ESP_LOGI(TAG_MAIN, "Serial = %s", devices[0].serial_number);

    vTaskDelay(10*1000 / portTICK_PERIOD_MS);

    vTaskSuspend(NULL);
}

