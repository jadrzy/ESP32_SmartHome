#include "pressure_sensor.h"
#include "driver/i2c_master.h"
#include "driver/i2c_types.h"
#include "esp_log.h"
#include <esp_check.h>
#include <stdint.h>
#include "freertos/idf_additions.h"
#include "i2c/i2c.h"

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


void pressure_sensor_config(i2c_master_dev_handle_t sensor_handle)
{
    // I2C transmission
    for (int i = 0; i < (sizeof(pressure_sensor_settings) / sizeof(pressure_sensor_settings[2])); i++)
    {
        ESP_ERROR_CHECK(i2c_master_transmit(   
            sensor_handle, 
            pressure_sensor_settings[i], 
            sizeof(pressure_sensor_settings[i]), 
            I2C_TIMEOUT_PRESSURE_SENSOR
        ));
    }
}


static void start_measurement(i2c_master_dev_handle_t sensor_handle)
{
    ESP_ERROR_CHECK(i2c_master_transmit(
        sensor_handle, 
        start_measurement_command, 
        sizeof(start_measurement_command), 
        I2C_TIMEOUT_PRESSURE_SENSOR
    ));
}


static void get_measurement(i2c_master_dev_handle_t sensor_handle)
{
    ESP_ERROR_CHECK(i2c_master_transmit(
        sensor_handle, 
        &get_measurement_command, 
        sizeof(get_measurement_command), 
        I2C_TIMEOUT_PRESSURE_SENSOR
    ));
    
    ESP_ERROR_CHECK(i2c_master_receive(
        sensor_handle, 
        pressure_measurement_bin, 
        sizeof(pressure_measurement_bin), 
        I2C_TIMEOUT_PRESSURE_SENSOR
    ));
}


void read_pressure_sensor(i2c_master_dev_handle_t sensor_handle)
{
    start_measurement(sensor_handle);
    vTaskDelay(PRESSURE_MEASUREMENT_DURATION);
    get_measurement(sensor_handle);
}
