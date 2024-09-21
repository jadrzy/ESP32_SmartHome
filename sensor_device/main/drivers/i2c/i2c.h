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

// I2C master configuration structure
static const i2c_master_bus_config_t i2c_master_config = {
    .clk_source = I2C_CLK_SRC_DEFAULT,  // Clock source
    .i2c_port = -1,  // Default I2C port 
    .scl_io_num = GPIO_NUM_22,  // SCL pin (clock)
    .sda_io_num = GPIO_NUM_21,  // SDA pin (data)
    .glitch_ignore_cnt = 7,     // Number of glitch pulses to ignore
    .flags.enable_internal_pullup = false,  // Disable internal pull-up resistors
};

// I2C device configuration for slave mode
static i2c_device_config_t i2c_slave_config = {
    .dev_addr_length = I2C_ADDR_BIT_7,  // Device address length (7-bit)
    .scl_speed_hz = 100000,  // I2C bus speed (100 kHz)
};

// Function declarations for I2C bus initialization
void i2c_init(struct i2c * i2c);

// Support functions for configuring I2C master and slave devices
static void master_init(i2c_master_bus_handle_t * bus);  // Initialize I2C master bus
static void slave_init(i2c_master_bus_handle_t * bus, i2c_master_dev_handle_t * sensor, uint8_t address); // Initialize I2C slave device

#endif
