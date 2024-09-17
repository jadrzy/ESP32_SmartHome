#include "pressure_sensor.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include <esp_check.h>
#include <stdint.h>
#include "freertos/idf_additions.h"
#include "i2c/i2c.h"

void pressure_sensor_config(i2c_master_dev_handle_t sensor_handle)
{
    // i2c transmission
    for (int i = 0; i < ( sizeof(command_pressure_sensor) / sizeof(command_pressure_sensor[2]) ); i++)
        ESP_ERROR_CHECK( i2c_master_transmit(   
            sensor_handle, command_pressure_sensor[i], sizeof(command_pressure_sensor[i]), I2C_TIMEOUT_PRESSURE_SENSOR));
}



void read_pressure_sensor(i2c_master_dev_handle_t sensor_handle)
{
    uint8_t command[] = {
        0xF4,
        0x15
    };

   // start measuring
    uint8_t command_read = 0xF3;
    uint8_t read;
    ESP_ERROR_CHECK(i2c_master_transmit(sensor_handle, &command_read, sizeof(command_read), I2C_TIMEOUT_PRESSURE_SENSOR));
    ESP_ERROR_CHECK(i2c_master_receive(sensor_handle, &read, sizeof(read), I2C_TIMEOUT_PRESSURE_SENSOR));
    ESP_LOGI("TAG", "status = %x", read);

    ESP_ERROR_CHECK(i2c_master_transmit(sensor_handle, command, sizeof(command), I2C_TIMEOUT_PRESSURE_SENSOR));

    ESP_ERROR_CHECK(i2c_master_transmit(sensor_handle, &command_read, sizeof(command_read), I2C_TIMEOUT_PRESSURE_SENSOR));
    ESP_ERROR_CHECK(i2c_master_receive(sensor_handle, &read, sizeof(read), I2C_TIMEOUT_PRESSURE_SENSOR));
    ESP_LOGI("TAG", "status = %x", read);

    vTaskDelay(40 / portTICK_PERIOD_MS);

    ESP_ERROR_CHECK(i2c_master_transmit(sensor_handle, &command_read, sizeof(command_read), I2C_TIMEOUT_PRESSURE_SENSOR));
    ESP_ERROR_CHECK(i2c_master_receive(sensor_handle, &read, sizeof(read), I2C_TIMEOUT_PRESSURE_SENSOR));
    ESP_LOGI("TAG", "status = %x", read);

    uint8_t read_val[3];
    command_read = 0xF7;
    ESP_ERROR_CHECK(i2c_master_transmit(sensor_handle, &command_read, sizeof(command_read), I2C_TIMEOUT_PRESSURE_SENSOR));
    ESP_ERROR_CHECK(i2c_master_receive(sensor_handle, &read_val[0], 1, I2C_TIMEOUT_PRESSURE_SENSOR));

    command_read = 0xF8;
    ESP_ERROR_CHECK(i2c_master_transmit(sensor_handle, &command_read, sizeof(command_read), I2C_TIMEOUT_PRESSURE_SENSOR));
    ESP_ERROR_CHECK(i2c_master_receive(sensor_handle, &read_val[1], 1, I2C_TIMEOUT_PRESSURE_SENSOR));
    command_read = 0xF9;

    ESP_ERROR_CHECK(i2c_master_transmit(sensor_handle, &command_read, sizeof(command_read), I2C_TIMEOUT_PRESSURE_SENSOR));
    ESP_ERROR_CHECK(i2c_master_receive(sensor_handle, &read_val[2], 1, I2C_TIMEOUT_PRESSURE_SENSOR));

    for (int i = 0; i < sizeof(read_val); i++)
        ESP_LOGI("TAG", "register[%d] = %x", i, read_val[i]);
    
}
