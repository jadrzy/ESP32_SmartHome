#include "driver/i2c_types.h"
#include <esp_log.h>
#include <stdint.h>
#include "i2c/i2c.h"

#ifndef HEADER_LIGHT_SENSOR_H
#define HEADER_LIGHT_SENSOR_H

#define I2C_TIMEOUT 100

static const uint8_t command[][2] = {
    
    // TIME
    {   0x81,       // TIMING REG
        0x10,   },  // GAIN = x1    INTEGRATION TIME = 402ms

    // INTERRUPT
    {   0x86,       // INTERRUPT REG
        0x00,   },  // DISABLE INTERRUPT

    // POWER UP
    {   0x80,       // CONTROL REG
        0x03,   },  // POWER ON

};

void light_sensor_config(i2c_master_dev_handle_t sensor_handle);
uint32_t read_light_sensor(i2c_master_dev_handle_t sensor_handle); 
unsigned int CalculateLux(unsigned int iGain, unsigned int tInt, unsigned int ch0, unsigned int ch1);

#endif
