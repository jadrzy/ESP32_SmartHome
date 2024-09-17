#include "driver/i2c_types.h"
#include "portmacro.h"
#include <esp_log.h>
#include <stdint.h>

#ifndef PRESSURE_SENSOR_H
#define PRESSURE_SENSOR_H

#define I2C_TIMEOUT_PRESSURE_SENSOR     100
#define PRESSURE_MEASUREMENT_DURATION   40 / portTICK_PERIOD_MS


static const uint8_t pressure_sensor_settings[][2] = {

    //  DATA 
    {   0xF4,       // CONTROL REG
        0x14,   },  // TMP_OVERSAMPLING OFF | PRESS_OVERSAMPLING x16 | SLEEP MODE

    // CONFIG
    {   0xF5,       // CONFIG REG
        0x10,   },  // IIR FILTER x16 

};

static const uint8_t start_measurement_command[2] = {
    0xF4,
    0x15
};

static const uint8_t get_measurement_command = 0xF7;

static uint8_t pressure_measurement_bin[3];



void pressure_sensor_setup(i2c_master_dev_handle_t sensor_handle);

static void start_measurement(i2c_master_dev_handle_t sensor_handle);
static void get_measurement(i2c_master_dev_handle_t sensor_handle);
void read_pressure_sensor(i2c_master_dev_handle_t sensor_handle); 

#endif
