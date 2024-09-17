#include "driver/i2c_types.h"
#include <esp_log.h>
#include <stdint.h>

#ifndef PRESSURE_SENSOR_H
#define PRESSURE_SENSOR_H

#define I2C_TIMEOUT_PRESSURE_SENSOR 100

static const uint8_t command_pressure_sensor[][2] = {
    
    //  DATA 
    {   0xF4,       // CONTROL REG
        0x14,   },  // TMP_OVERSAMPLING OFF | PRESS_OVERSAMPLING x16 | SLEEP MODE

    // CONFIG
    {   0xF5,       // CONFIG REG
        0x10,   },  // IIR FILTER x16 
   
};

void pressure_sensor_config(i2c_master_dev_handle_t sensor_handle);
void read_pressure_sensor(i2c_master_dev_handle_t sensor_handle); 

#endif
