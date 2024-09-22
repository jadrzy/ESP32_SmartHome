#include <driver/i2c_types.h>
#include <driver/i2c_master.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <stdint.h>

#ifndef HEADER_I2C_H
#define HEADER_I2C_H

// I2C device addresses
#define LIGHT_SENSOR_ADDRESS    0x29  // Light sensor address
#define TEMP_HUM_SENSOR_ADDRESS 0x38  // Temperature and humidity sensor address
#define PRESSURE_SENSOR_ADDRESS 0x77  // Pressure sensor address

// Structure to store I2C initialization status and device handles
struct i2c {
    i2c_master_bus_handle_t bus_handle;  // I2C bus handle
    i2c_master_dev_handle_t light_sensor_handle,    // Handle for light sensor
                            temp_hum_sensor_handle, // Handle for temperature and humidity sensor
                            pressure_sensor_handle; // Handle for pressure sensor
};

// Function declarations for I2C bus initialization
void i2c_init(struct i2c * i2c);

// Support functions for configuring I2C master and slave devices
void master_init(i2c_master_bus_handle_t * bus);  // Initialize I2C master bus
void slave_init(i2c_master_bus_handle_t * bus, i2c_master_dev_handle_t * sensor, uint8_t address); // Initialize I2C slave device

#endif
