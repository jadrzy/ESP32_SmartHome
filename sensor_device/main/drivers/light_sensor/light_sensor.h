#include <driver/i2c_types.h>
#include <driver/i2c_master.h>
#include <esp_check.h>
#include <esp_log.h>
#include <stdint.h>

#ifndef HEADER_LIGHT_SENSOR_H
#define HEADER_LIGHT_SENSOR_H

// Timeout for I2C transactions related to the light sensor (in milliseconds)
#define I2C_TIMEOUT_LIGHT_SENSOR 100

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

// Function declarations
void light_sensor_config(i2c_master_dev_handle_t * sensor_handle);  // Configure the light sensor
void read_light_sensor(i2c_master_dev_handle_t * sensor_handle, uint32_t * lux);  // Read light sensor data
static unsigned int CalculateLux(unsigned int iGain, unsigned int tInt, unsigned int ch0, unsigned int ch1);  // Calculate lux from raw sensor data

#endif
