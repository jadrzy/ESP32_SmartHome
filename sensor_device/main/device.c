#include "device.h"
#include "i2c/i2c.h"
void app_main(void)

{
    ESP_LOGI("TAG", "app_started");
    i2c_init();
    sensor_config();        
}
