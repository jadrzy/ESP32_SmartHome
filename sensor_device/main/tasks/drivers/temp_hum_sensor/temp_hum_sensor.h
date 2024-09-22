#include "driver/i2c_types.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include <stdint.h>

#ifndef HEADER_TEMP_HUM_SENSOR_H
#define HEADER_TEMP_HUM_SENSOR_H

#define I2C_TIMEOUT_TEMP_HUM_SENSOR 100

double iir_filter_double(double data_new, double data_old, uint8_t filter_coeff);
void temp_hum_sensor_config(i2c_master_dev_handle_t * sensor_handle);
void read_temp_hum_sensor(i2c_master_dev_handle_t * sensor_handle, double * humidity_ptr, double * temp_ptr); 

#endif
