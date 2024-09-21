#include "driver/i2c_types.h"
#include <driver/i2c_master.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <stdint.h>

#ifndef HEADER_I2C_H
#define HEADER_I2C_H

#define LIGHT_SENSOR_ADDRESS    0x29
#define TEMP_HUM_SENSOR_ADDRESS 0x38
#define PRESSURE_SENSOR_ADDRESS 0x77

struct i2c {
    uint8_t init_status; 
    i2c_master_bus_handle_t bus_handle;
    i2c_master_dev_handle_t light_sensor_handle,
                            temp_hum_sensor_handle,
                            pressure_sensor_handle;
};

static const i2c_master_bus_config_t i2c_master_config = {
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .i2c_port = -1,
    .scl_io_num = GPIO_NUM_22,
    .sda_io_num = GPIO_NUM_21,
    .glitch_ignore_cnt = 7,
    .flags.enable_internal_pullup = false,
};


static i2c_device_config_t i2c_slave_config = {
    .dev_addr_length = I2C_ADDR_BIT_7,
    .scl_speed_hz = 100000,
};

void i2c_init(struct i2c * i2c);
static void master_init(i2c_master_bus_handle_t * bus);
static void slave_init(i2c_master_bus_handle_t * bus, i2c_master_dev_handle_t * sensor, uint8_t address);

#endif
