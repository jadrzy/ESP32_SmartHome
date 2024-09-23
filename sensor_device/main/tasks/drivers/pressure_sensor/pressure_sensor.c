#include "pressure_sensor.h"
#include "driver/i2c_master.h"
#include "driver/i2c_types.h"
#include "esp_log.h"
#include <esp_check.h>
#include <stdint.h>
#include "freertos/idf_additions.h"

static struct data_calibration d_calib = {
    .T1 = 27504,
    .T2 = 25435,
    .T3 = -1000,
    .P1 = 36477,
    .P2 = -10685,
    .P3 = 3024,
    .P4 = 2855,
    .P5 = 140,
    .P6 = -7,
    .P7 = 15500,
    .P8 = -14600,
    .P9 = 6000
};

void pressure_sensor_config(i2c_master_dev_handle_t * sensor_handle)
{
    const uint8_t pressure_sensor_settings[][2] = {
        {   0xF4,
            0x14,},
        // CONFIG
        {   0xF5,       // CONFIG REG
            0x10,   },  // IIR FILTER x16 
    };
    // I2C transmission
    for (int i = 0; i < (sizeof(pressure_sensor_settings) / sizeof(pressure_sensor_settings[2])); i++)
    {
        ESP_ERROR_CHECK(i2c_master_transmit(   
            *sensor_handle, 
            pressure_sensor_settings[i], 
            sizeof(pressure_sensor_settings[i]), 
            I2C_TIMEOUT_PRESSURE_SENSOR
        ));
    }
    // get_calibration_data(sensor_handle, &d_calib);
}

void get_calibration_data(i2c_master_dev_handle_t * sensor_handle, struct data_calibration * data)
{
    uint8_t buffer[24];
    const uint8_t get_measurement_command = 0x88;
    ESP_ERROR_CHECK(i2c_master_transmit(
        *sensor_handle, 
        &get_measurement_command, 
        sizeof(get_measurement_command), 
        I2C_TIMEOUT_PRESSURE_SENSOR
    ));
    
    ESP_ERROR_CHECK(i2c_master_receive(
        *sensor_handle, 
        buffer, 
        sizeof(buffer), 
        I2C_TIMEOUT_PRESSURE_SENSOR
    ));
    data->T1 = buffer[0] | (buffer[1] << 8);
    data->T2 = buffer[2] | (buffer[3] << 8);
    data->T3 = buffer[4] | (buffer[5] << 8);
    data->P1 = buffer[6] | (buffer[7] << 8);
    data->P2 = buffer[8] | (buffer[9] << 8);
    data->P3 = buffer[10] | (buffer[11] << 8);
    data->P4 = buffer[12] | (buffer[13] << 8);
    data->P5 = buffer[14] | (buffer[15] << 8);
    data->P6 = buffer[16] | (buffer[17] << 8);
    data->P7 = buffer[18] | (buffer[19] << 8);
    data->P8 = buffer[20] | (buffer[21] << 8);
    data->P9 = buffer[22] | (buffer[23] << 8);
}

double calculate_pressure(int temp, int press)
{
    temp = 519888;
    press = 415148;
    int var1, var2;
    uint32_t p;
    var1 = ((((temp>>3)-((int)d_calib.T1<<1)))*((int)d_calib.T2))>>11;
    var2 = (((((temp>>4)-((int)d_calib.T1))*((temp>>4)-((int)d_calib.T1)))>>12)*((int)d_calib.T3))>>14;
    temp = var1+var2;

    var1 = (((int)temp)>>1)-(int)64000;
    var2 = (((var1>>2)*(var1>>2))>>11)*((int)d_calib.P6);
    var2 = var2+((var1*((int)d_calib.P5))<<1);
    var2 = (var2>>2)+(((int)d_calib.P4)<<16);
    var1 = (((d_calib.P3*(((var1>>2)*(var1>>2))>>13))>>3)+((((int)d_calib.P2)*var1)>>1))>>18;
    var1 = ((((32768+var1))*((int)d_calib.P1))>>15);
    if (var1 == 0)
    {
        p = 0;
    }
    else
    {
        p = (((uint32_t)(((int)1038576)-press)-(var2>>12)))*3125;
        if (p < 0x80000000)
        {
            p = (p << 1) / ((uint32_t)var1);
        }
        else
        {
            p = (p / (uint32_t)var1)*2;
        }
        var1 = (((int)d_calib.P9)*((int)(((p>>3)*(p>>3))>>13)))>>12;
        var2 = (((int)(p>>2))*((int)d_calib.P8))>>13;
        p = (uint32_t)((int)p+((var1+var2+d_calib.P7)>>4));
    }
    return ((double)p / 100);
}

void start_measurement(i2c_master_dev_handle_t * sensor_handle)
{
    const uint8_t start_measurement_command[] = {
        0xF4,
        0xB5
    };
    ESP_ERROR_CHECK(i2c_master_transmit(
        *sensor_handle, 
        start_measurement_command, 
        sizeof(start_measurement_command), 
        I2C_TIMEOUT_PRESSURE_SENSOR
    ));
}


double get_measurement(i2c_master_dev_handle_t * sensor_handle)
{
    uint8_t measurement_bin[6];
    const uint8_t get_measurement_command = 0xF7;
    ESP_ERROR_CHECK(i2c_master_transmit(
        *sensor_handle, 
        &get_measurement_command, 
        sizeof(get_measurement_command), 
        I2C_TIMEOUT_PRESSURE_SENSOR
    ));
    
    ESP_ERROR_CHECK(i2c_master_receive(
        *sensor_handle, 
        measurement_bin, 
        sizeof(measurement_bin), 
        I2C_TIMEOUT_PRESSURE_SENSOR
    ));
    int pressure_bin = (measurement_bin[0] << 12) | (measurement_bin[1] << 4) | (measurement_bin[2] >> 4);
    int temperature_bin = (measurement_bin[3] << 12) | (measurement_bin[4] << 4) | (measurement_bin[5] >> 4);

    return calculate_pressure(temperature_bin, pressure_bin);
}


void read_pressure_sensor(i2c_master_dev_handle_t * sensor_handle, double * pressure)
{
    start_measurement(sensor_handle);
    vTaskDelay(PRESSURE_MEASUREMENT_DURATION);
    *pressure = get_measurement(sensor_handle);
}
