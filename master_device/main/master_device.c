#include "master_device.h"
#include "esp_log.h"
#include "nvs/nvs.h"
#include "nvs_flash.h"
#include <stdio.h>
#include <string.h>

static char master_serial_number[SERIAL_NUMBER_SIZE + 1];
static char slave_serial_number[NUMBER_OF_DEVICES][SERIAL_NUMBER_SIZE + 1];

void app_main(void)
{
    init_nvs_partitions();
    get_master_serial_number_from_nvs(master_serial_number);
    get_slave_serial_number_from_nvs(slave_serial_number);
    ESP_LOGI("TAG", "MASTER_SERIAL = %s", master_serial_number);

    for (int i = 0; i < NUMBER_OF_DEVICES; i++)
    {
        ESP_LOGI("TAG", "SLAVE_%02d_SERIAL = %s", (i + 1), slave_serial_number[i]);
    }

}
