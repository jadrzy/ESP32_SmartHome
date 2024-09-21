#include "device.h"


static struct i2c i2c;
static struct data data;

void app_main(void)
{
    ESP_LOGI("TAG", "app_started");
    i2c_init(&i2c);


    while (1)
    {
        light_sensor_config(&i2c.light_sensor_handle);
        read_light_sensor(&i2c.light_sensor_handle, &data.lux);

        ESP_LOGI("TAG", "Lux = %d", (int) data.lux);
        vTaskDelay(15 / portTICK_PERIOD_MS);
    }
}
