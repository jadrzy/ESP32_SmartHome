#include "temp_hum_sensor.h"
#include "freertos/idf_additions.h"
#include <stdint.h>

static const uint8_t command_temp_hum_sensor[] = {
    // SETUP COMMAND (CALIBRATION)
    0xBE,   // command
    0x08,   // parameter 1
    0x00    // parameter 2
};

void temp_hum_sensor_config(i2c_master_dev_handle_t * sensor_handle)
{
    // i2c transmission
    ESP_ERROR_CHECK(i2c_master_transmit(
        *sensor_handle, 
        command_temp_hum_sensor, 
        sizeof(command_temp_hum_sensor), 
        I2C_TIMEOUT_TEMP_HUM_SENSOR
    ));
}

double iir_filter_double(double data_new, double data_old, uint8_t filter_coeff)
{
    return ( data_old * (filter_coeff - 1) + data_new ) / filter_coeff;
}

void read_temp_hum_sensor(i2c_master_dev_handle_t * sensor_handle, double * humidity, double * temperature)
{
    uint8_t command[] = {
        0xAC,
        0x33,
        0x00
    };
    // Start measurement
    ESP_ERROR_CHECK(i2c_master_transmit(
        *sensor_handle, 
        command, 
        sizeof(command), 
        I2C_TIMEOUT_TEMP_HUM_SENSOR
    ));

    // Wait until measurement is finished
    vTaskDelay(85 / portTICK_PERIOD_MS);

    // Get measurement 
    uint8_t data_recieved[6] = {0}; 
    while(1)
    {
        ESP_ERROR_CHECK(i2c_master_receive(
            *sensor_handle, 
            data_recieved, 
            sizeof(data_recieved), 
            I2C_TIMEOUT_TEMP_HUM_SENSOR
        ));

        if(data_recieved[0] & 0x80)
        {
            ESP_LOGI("TAG", "Error reading, sensor is busy");
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        else if(!(data_recieved[0] & 0x08))
        {
            ESP_LOGI("TAG", "Error reading, sensor is uncalibrated");
            temp_hum_sensor_config(sensor_handle);
            break; // Leave loop and display message if sensor is noc callibrated
        }   
        else
        {
            break; // Leave loop if data was read correctly
        }
    }


    uint32_t binary_humidity = (data_recieved[1] << 12) | (data_recieved[2] << 4) | (data_recieved[3] >> 4); 
    uint32_t binary_temperature = ((data_recieved[3] & 0xF) << 16) | (data_recieved[4] << 8) | data_recieved[5]; 

    double new_humidity = ((double) binary_humidity / 1048567) * 100;
    double new_temperature = ((double) binary_temperature / 1048567) * 200 - 50;

    *humidity = iir_filter_double(new_humidity, *humidity, 8); 
    *temperature = iir_filter_double(new_temperature, *temperature, 8); 
}


