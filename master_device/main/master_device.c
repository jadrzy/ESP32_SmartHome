#include "master_device.h"
#include "esp_err.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <stdint.h>

static uint64_t assigned_devices[10] = {0};


void init_nvs(uint64_t * dev)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}


void app_main(void)
{

}
