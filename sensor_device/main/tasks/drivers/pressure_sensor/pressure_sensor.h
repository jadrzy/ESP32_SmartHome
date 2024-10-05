#include "driver/i2c_types.h"
#include <esp_log.h>
#include <stdint.h>

#ifndef PRESSURE_SENSOR_H
#define PRESSURE_SENSOR_H

// Timeout and duration definitions
#define I2C_TIMEOUT_PRESSURE_SENSOR   100                             // I2C communication timeout (ms)
#define PRESSURE_MEASUREMENT_DURATION (45 / portTICK_PERIOD_MS)       // Pressure measurement duration

// Structure to hold pressure sensor calibration data
struct data_calibration {
    unsigned short T1;    // Temperature calibration coefficient 1
    short T2;             // Temperature calibration coefficient 2
    short T3;             // Temperature calibration coefficient 3
    unsigned short P1;    // Pressure calibration coefficient 1
    short P2;             // Pressure calibration coefficient 2
    short P3;             // Pressure calibration coefficient 3
    short P4;             // Pressure calibration coefficient 4
    short P5;             // Pressure calibration coefficient 5
    short P6;             // Pressure calibration coefficient 6
    short P7;             // Pressure calibration coefficient 7
    short P8;             // Pressure calibration coefficient 8
    short P9;             // Pressure calibration coefficient 9
};

// Function to configure the pressure sensor
void pressure_sensor_config(i2c_master_dev_handle_t *sensor_handle);

// Function to retrieve calibration data from the sensor
void get_calibration_data(i2c_master_dev_handle_t *sensor_handle, struct data_calibration *data);

// Function to calculate the pressure based on temperature and pressure readings
double calculate_pressure(int temp, int press);

// Function to start a pressure measurement
void start_measurement(i2c_master_dev_handle_t *sensor_handle);

// Function to get the pressure measurement result
double get_measurement(i2c_master_dev_handle_t *sensor_handle);

// High-level function to read the pressure from the sensor
void read_pressure_sensor(i2c_master_dev_handle_t *sensor_handle, double *pressure);

#endif  // PRESSURE_SENSOR_H
