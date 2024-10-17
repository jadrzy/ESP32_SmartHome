#include "master_device.h"
#include "esp_log.h"
#include "components/nvs/nvs.h"
#include "components/wifi/wifi.h"
#include "nvs_flash.h"
#include <stdio.h>
#include <string.h>

void app_main(void)
{
    ESP_ERROR_CHECK(memory_setup());
    ESP_ERROR_CHECK(wifi_init(true));
}
