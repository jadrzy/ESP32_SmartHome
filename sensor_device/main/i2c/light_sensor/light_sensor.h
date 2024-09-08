#include "driver/i2c_master.h"
#include "driver/i2c_types.h"
#include <esp_log.h>
#include <stdint.h>

#ifndef HEADER_LIGHT_SENSOR_H
#define HEADER_LIGHT_SENSOR_H

#define I2C_TIMEOUT 100

static const uint8_t command[][2] = {
    
    // TIME
    {   0x81,       // TIMING REG
        0x02,   },  // GAIN = x1    INTEGRATION TIME = 402ms

    // INTERRUPT
    {   0x86,       // INTERRUPT REG
        0x00,   },  // DISABLE INTERRUPT

    // POWER UP
    {   0x80,       // CONTROL REG
        0x03,   },  // POWER ON

};

void light_sensor_config(i2c_master_dev_handle_t sensor_handle);
double read_light_sensor(i2c_master_dev_handle_t sensor_handle); 

#endif
