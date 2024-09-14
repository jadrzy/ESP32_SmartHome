#ifndef HEADER_I2C_H
#define HEADER_I2C_H

#include "driver/i2c_types.h"
#include <driver/i2c_master.h>
#include <stdint.h>

#define LIGHT_SENSOR_ADDRESS 0x29

static i2c_master_bus_handle_t bus_handle;
static i2c_master_dev_handle_t light_sensor_handle;
// static i2c_master_dev_handle_t temp_sensor_handle;

void i2c_init(void);
void sensor_config(void);
void i2c_read_sensors(uint8_t * prt_lux);
i2c_master_bus_handle_t master_bus_init(void);
i2c_master_dev_handle_t slave_dev_init(uint16_t dev_address, i2c_master_bus_handle_t master_bus_handle);

#endif
