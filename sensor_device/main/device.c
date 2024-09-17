#include "device.h"
#include "freertos/idf_additions.h"
#include "i2c/i2c.h"
#include <stdint.h>


void app_main(void)
{
    ESP_LOGI("TAG", "app_started");
    // static struct Data data;
    i2c_init();

}
