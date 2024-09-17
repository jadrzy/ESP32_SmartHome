#include "temp_hum_sensor.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include <stdint.h>

void temp_hum_sensor_config(i2c_master_dev_handle_t sensor_handle)
{
    // i2c transmission
    ESP_ERROR_CHECK(i2c_master_transmit(sensor_handle, command_temp_hum_sensor, sizeof(command_temp_hum_sensor), I2C_TIMEOUT_TEMP_HUM_SENSOR));
}

void read_temp_hum_sensor(i2c_master_dev_handle_t sensor_handle)
{
    uint8_t command[] = {
        0xAC,
        0x33,
        0x00
    };
    ESP_ERROR_CHECK(i2c_master_transmit(sensor_handle, command, sizeof(command), I2C_TIMEOUT_TEMP_HUM_SENSOR));

    vTaskDelay(100 / portTICK_PERIOD_MS);

    uint8_t data_recieved[6]; 
    ESP_ERROR_CHECK(i2c_master_receive(sensor_handle, data_recieved, sizeof(data_recieved), I2C_TIMEOUT_TEMP_HUM_SENSOR));
    for (int i = 0; i < sizeof(data_recieved); i++)
    {
        ESP_LOGI("TAG", "data_recieved[%d]: %x", i, data_recieved[i]);
    }

    if (data_recieved[0] & 0x80)
        ESP_LOGI("TAG", "Error reading, sensor is busy");
    if (!(data_recieved[0] & 0x08))
        ESP_LOGI("TAG", "Error reading, sensor is uncalibrated");

    uint32_t humidity = (data_recieved[1] << 12) | (data_recieved[2] << 4) | (data_recieved[3] >> 4); 
    uint32_t temperature = ((data_recieved[3] & 0xF) << 16) | (data_recieved[4] << 8) | data_recieved[5]; 

    double real_humidity = ( (double) humidity / 1048567) * 100;
    double real_temperature = ( (double) temperature / 1048567) * 200 - 50;

    ESP_LOGI("TAG", "Humidity = %f", real_humidity);
    ESP_LOGI("TAG", "Temperature = %f", real_temperature);
}
