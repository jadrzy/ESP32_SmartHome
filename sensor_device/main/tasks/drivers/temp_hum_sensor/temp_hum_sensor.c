#include "temp_hum_sensor.h"

// Setup command for the temperature and humidity sensor (calibration)
static const uint8_t command_temp_hum_sensor[] = {
    0xBE,   // Command byte
    0x08,   // Parameter 1
    0x00    // Parameter 2
};

// Configures the temperature and humidity sensor via I2C communication.
void temp_hum_sensor_config(i2c_master_dev_handle_t *sensor_handle)
{
    // Transmit setup command to the sensor
    ESP_ERROR_CHECK(i2c_master_transmit(
        *sensor_handle, 
        command_temp_hum_sensor, 
        sizeof(command_temp_hum_sensor), 
        I2C_TIMEOUT_TEMP_HUM_SENSOR
    ));
}

// Applies an Infinite Impulse Response (IIR) filter to smooth sensor data.
double iir_filter_double(double data_new, double data_old, uint8_t filter_coeff)
{
    return (data_old * (filter_coeff - 1) + data_new) / filter_coeff;
}

// Reads temperature and humidity data from the sensor.
void read_temp_hum_sensor(i2c_master_dev_handle_t *sensor_handle, double *humidity, double *temperature)
{
    // Command to start a measurement
    uint8_t command[] = { 0xAC, 0x33, 0x00 };

    // Start measurement by sending the command to the sensor
    ESP_ERROR_CHECK(i2c_master_transmit(
        *sensor_handle, 
        command, 
        sizeof(command), 
        I2C_TIMEOUT_TEMP_HUM_SENSOR
    ));

    // Delay for the measurement duration (85 ms)
    vTaskDelay(85 / portTICK_PERIOD_MS);

    // Buffer to store received data
    uint8_t data_received[6] = {0};

    // Loop until valid data is received
    while (1)
    {
        // Receive data from the sensor
        ESP_ERROR_CHECK(i2c_master_receive(
            *sensor_handle, 
            data_received, 
            sizeof(data_received), 
            I2C_TIMEOUT_TEMP_HUM_SENSOR
        ));

        // Check if the sensor is busy (MSB indicates busy status)
        if (data_received[0] & 0x80)
        {
            ESP_LOGI("TAG", "Error reading: sensor is busy");
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        // Check if the sensor is uncalibrated (status bit 3 indicates calibration)
        else if (!(data_received[0] & 0x08))
        {
            ESP_LOGI("TAG", "Error reading: sensor is uncalibrated");
            temp_hum_sensor_config(sensor_handle);  // Recalibrate the sensor
            break;  // Exit loop after recalibration
        }
        else
        {
            break;  // Exit loop if data is valid
        }
    }

    // Extract humidity and temperature from the received data
    uint32_t binary_humidity = (data_received[1] << 12) | (data_received[2] << 4) | (data_received[3] >> 4);
    uint32_t binary_temperature = ((data_received[3] & 0x0F) << 16) | (data_received[4] << 8) | data_received[5];

    // Convert binary data to percentage (humidity) and Celsius (temperature)
    double new_humidity = ((double)binary_humidity / 1048567.0) * 100.0;
    double new_temperature = ((double)binary_temperature / 1048567.0) * 200.0 - 50.0;

    // Apply IIR filter to smooth the readings
    *humidity = iir_filter_double(new_humidity, *humidity, 8);
    *temperature = iir_filter_double(new_temperature, *temperature, 8);
}
