#include "device.h"
#include "freertos/idf_additions.h"
#include "nvs/nvs.h"

static char slave_serial_number[SERIAL_NUMBER_SIZE + 1];

void app_main(void)
{
    ESP_LOGI("TAG", "app_started");
    init_nvs_partitions();
    get_slave_serial_number_from_nvs(slave_serial_number);

    ESP_LOGI("TAG", "Slave serial number = %s", slave_serial_number);

    vTaskDelay(100 / portTICK_PERIOD_MS);
    initialize_sensors();    
    vTaskDelay(100 / portTICK_PERIOD_MS);
    initialize_tasks();
    vTaskSuspend(NULL);
}
