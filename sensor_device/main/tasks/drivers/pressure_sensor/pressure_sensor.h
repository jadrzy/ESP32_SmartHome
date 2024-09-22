#include "driver/i2c_types.h"
#include <esp_log.h>
#include <stdint.h>

#ifndef PRESSURE_SENSOR_H
#define PRESSURE_SENSOR_H

#define I2C_TIMEOUT_PRESSURE_SENSOR   100
#define PRESSURE_MEASUREMENT_DURATION (40 / portTICK_PERIOD_MS)


void pressure_sensor_setup(i2c_master_dev_handle_t sensor_handle);

static void start_measurement(i2c_master_dev_handle_t sensor_handle);
static void get_measurement(i2c_master_dev_handle_t sensor_handle);
void read_pressure_sensor(i2c_master_dev_handle_t sensor_handle); 

#endif
