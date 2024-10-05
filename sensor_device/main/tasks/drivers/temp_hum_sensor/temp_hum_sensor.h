#ifndef HEADER_TEMP_HUM_SENSOR_H
#define HEADER_TEMP_HUM_SENSOR_H

#include "driver/i2c_types.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include <stdint.h>

// Define the I2C communication timeout (in milliseconds) for the temperature and humidity sensor
#define I2C_TIMEOUT_TEMP_HUM_SENSOR 100

// Applies an Infinite Impulse Response (IIR) filter to smooth out the sensor data.
double iir_filter_double(double data_new, double data_old, uint8_t filter_coeff);

// Initializes and configures the temperature and humidity sensor using I2C communication.
void temp_hum_sensor_config(i2c_master_dev_handle_t *sensor_handle);

// Reads the temperature and humidity values from the sensor.
void read_temp_hum_sensor(i2c_master_dev_handle_t *sensor_handle, double *humidity_ptr, double *temp_ptr);

#endif // HEADER_TEMP_HUM_SENSOR_H
