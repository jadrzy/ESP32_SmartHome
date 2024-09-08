#include "light_sensor.h"
#include "esp_log.h"

void light_sensor_config(i2c_master_dev_handle_t sensor_handle)
{
    for (int i = 0; i < ( sizeof(command) / sizeof(command[2]) ); i++)
        ESP_ERROR_CHECK(i2c_master_transmit(sensor_handle, command[i], sizeof(command[i]), I2C_TIMEOUT));
}

double read_light_sensor(i2c_master_dev_handle_t sensor_handle)
{
    uint8_t command,
            channel_0[2],
            channel_1[2];

    command = 0xAC;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(sensor_handle, &command, 1, channel_0, 2, I2C_TIMEOUT));
    ESP_LOGI("TAG", "CH0 = %x, %x", channel_0[0], channel_0[1]);

    command = 0xAE;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(sensor_handle, &command, 1, channel_1, 2, I2C_TIMEOUT));
    ESP_LOGI("TAG", "CH1 = %x, %x", channel_1[0], channel_1[1]);


    return 0;
}
