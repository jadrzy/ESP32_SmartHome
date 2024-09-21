#include "light_sensor.h"

// LIGHT SENSOR INITIAL SETUP
void light_sensor_config(i2c_master_dev_handle_t * sensor_handle)
{
    // i2c transmission
    for (int i = 0; i < (sizeof(command_light_sensor) / sizeof(command_light_sensor[2])); i++)
        ESP_ERROR_CHECK(i2c_master_transmit(
            *sensor_handle, 
            command_light_sensor[i], 
            sizeof(command_light_sensor[i]), 
            I2C_TIMEOUT_LIGHT_SENSOR
        ));
}

// LIGHT SENSOR READ VALUE FUNCTION
void read_light_sensor(i2c_master_dev_handle_t * sensor_handle, uint32_t * lux)
{
    uint8_t command,
            channel_0[2],
            channel_1[2];

    // i2c read photodiode 0 register values
    command = 0xAC;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(
        *sensor_handle, 
        &command, 
        1, 
        channel_0, 
        sizeof(channel_0), 
        I2C_TIMEOUT_LIGHT_SENSOR
    ));

    // i2c read photodiode 1 register values
    command = 0xAE;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(
        *sensor_handle, 
        &command, 
        1, 
        channel_1, 
        sizeof(channel_1), 
        I2C_TIMEOUT_LIGHT_SENSOR
    ));

    // combine values 
    uint16_t channel_0_sum, channel_1_sum; 
    channel_0_sum = (channel_0[1] << 8) | channel_0[0];
    channel_1_sum = (channel_1[1] << 8) | channel_1[0];

    // convert photodiodes readings to lighting intensity [lux]
    *lux = CalculateLux(1, 0, channel_0_sum, channel_1_sum); 
}
