#include "light_sensor.h"

// Command sequence for configuring the light sensor over I2C
static const uint8_t command_light_sensor[][2] = {
    
    // Timing configuration
    {   
        0x81,   // Timing register
        0x10,   // GAIN = 1x, INTEGRATION TIME = 402ms
    },  

    // Interrupt configuration
    {   
        0x86,   // Interrupt register
        0x00,   // Disable interrupts
    },

    // Power up the sensor
    {   
        0x80,   // Control register
        0x03,   // Power on
    },
};

//****************************************************************************
// Function: light_sensor_config
// Description: Configures the light sensor by sending a series of initialization 
// commands over I2C communication.
//y Arguments:
//    sensor_handle - Handle to the I2C sensor device.
//****************************************************************************
void light_sensor_config(i2c_master_dev_handle_t * sensor_handle)
{
    // Loop through all light sensor commands and send them via I2C
    for (int i = 0; i < (sizeof(command_light_sensor) / sizeof(command_light_sensor[2])); i++) 
    {
        ESP_ERROR_CHECK(i2c_master_transmit(
            *sensor_handle, 
            command_light_sensor[i], 
            sizeof(command_light_sensor[i]), 
            I2C_TIMEOUT_LIGHT_SENSOR
        ));
    }
}

//****************************************************************************
// Function: read_light_sensor
// Description: Reads raw data from the light sensor's photodiodes and converts 
// it into a lux value (lighting intensity).
// Arguments:
//    sensor_handle - Handle to the I2C sensor device.
//    lux - Pointer to store the calculated lux value.
//****************************************************************************
void read_light_sensor(i2c_master_dev_handle_t * sensor_handle, uint32_t * lux)
{
    uint8_t command;
    uint8_t channel_0[2], channel_1[2];  // Buffers to store raw channel data

    // Read photodiode 0 (visible light focused) register values over I2C
    command = 0xAC;  // Command to read from photodiode 0
    ESP_ERROR_CHECK(i2c_master_transmit_receive(
        *sensor_handle, 
        &command, // Request photodiode 0 data 
        1,           
        channel_0, 
        sizeof(channel_0), 
        I2C_TIMEOUT_LIGHT_SENSOR
    ));

    // Read photodiode 1 (IR focused) register values over I2C
    command = 0xAE;  // Command to read from photodiode 1
    ESP_ERROR_CHECK(i2c_master_transmit_receive(
        *sensor_handle, 
        &command, // Request photodiode 1 data 
        1, 
        channel_1, 
        sizeof(channel_1), 
        I2C_TIMEOUT_LIGHT_SENSOR
    ));

    // Combine two bytes from each photodiode to form a 16-bit value
    uint16_t channel_0_sum = (channel_0[1] << 8) | channel_0[0];
    uint16_t channel_1_sum = (channel_1[1] << 8) | channel_1[0];

    // Convert the raw photodiode readings into a lux value
    *lux = CalculateLux(1, 0, channel_0_sum, channel_1_sum);
}
