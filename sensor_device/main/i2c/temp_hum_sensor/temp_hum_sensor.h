#include "driver/i2c_types.h"
#include "i2c/i2c.h"

#ifndef HEADER_TEMP_HUM_SENSOR_H
#define HEADER_TEMP_HUM_SENSOR_H

#define I2C_TIMEOUT_TEMP_HUM_SENSOR 100

static const uint8_t command_temp_hum_sensor[] = {
    // SETUP COMMAND (CALIBRATION)
    0xBE,   // command
    0x08,   // parameter 1
    0x00    // parameter 2
};

void temp_hum_sensor_config(i2c_master_dev_handle_t sensor_handle);
void read_temp_hum_sensor(i2c_master_dev_handle_t sensor_handle); 

#endif
