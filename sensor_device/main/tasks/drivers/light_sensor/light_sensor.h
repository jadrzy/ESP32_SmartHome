#include <driver/i2c_types.h>
#include <driver/i2c_master.h>
#include <esp_check.h>
#include <esp_log.h>
#include <stdint.h>

#ifndef HEADER_LIGHT_SENSOR_H
#define HEADER_LIGHT_SENSOR_H

// Timeout for I2C transactions related to the light sensor (in milliseconds)
#define I2C_TIMEOUT_LIGHT_SENSOR 100

// Function declarations
void light_sensor_config(i2c_master_dev_handle_t * sensor_handle);  // Configure the light sensor
void read_light_sensor(i2c_master_dev_handle_t * sensor_handle, uint32_t * lux);  // Read light sensor data
unsigned int CalculateLux(unsigned int iGain, unsigned int tInt, unsigned int ch0, unsigned int ch1);  // Calculate lux from raw sensor data

#endif
