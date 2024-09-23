#include "driver/i2c_types.h"
#include <esp_log.h>
#include <stdint.h>

#ifndef PRESSURE_SENSOR_H
#define PRESSURE_SENSOR_H

#define I2C_TIMEOUT_PRESSURE_SENSOR   100
#define PRESSURE_MEASUREMENT_DURATION (45 / portTICK_PERIOD_MS)

struct data_calibration {
    unsigned short T1;
    short T2;
    short T3;
    unsigned short P1;
    short P2;
    short P3;
    short P4;
    short P5;
    short P6;
    short P7;
    short P8;
    short P9;
};

void pressure_sensor_config(i2c_master_dev_handle_t * sensor_handle);
void get_calibration_data(i2c_master_dev_handle_t * sensor_handle, struct data_calibration * data);
double calculate_pressure(int temp, int press);

void start_measurement(i2c_master_dev_handle_t * sensor_handle);
double get_measurement(i2c_master_dev_handle_t * sensor_handle);
void read_pressure_sensor(i2c_master_dev_handle_t * sensor_handle, double * pressure); 

#endif
