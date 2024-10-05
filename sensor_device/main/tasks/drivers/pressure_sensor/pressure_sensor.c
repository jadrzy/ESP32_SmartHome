#include "pressure_sensor.h"

// Structure to hold calibration data
static struct data_calibration d_calib;

// Function to configure the pressure sensor
void pressure_sensor_config(i2c_master_dev_handle_t *sensor_handle)
{
    // Settings for the pressure sensor configuration
    const uint8_t pressure_sensor_settings[] = {
        0xF5, // CONFIG REG
        0x10  // IIR FILTER x16
    };
    
    // I2C transmission to configure the sensor
    ESP_ERROR_CHECK(i2c_master_transmit(
        *sensor_handle, 
        pressure_sensor_settings, 
        sizeof(pressure_sensor_settings), 
        I2C_TIMEOUT_PRESSURE_SENSOR
    ));

    // Retrieve calibration data from the sensor
    get_calibration_data(sensor_handle, &d_calib);
}

// Function to get calibration data from the sensor
void get_calibration_data(i2c_master_dev_handle_t *sensor_handle, struct data_calibration *data)
{
    uint8_t buffer[24];
    const uint8_t get_measurement_command = 0x88;

    // Request calibration data from the sensor
    ESP_ERROR_CHECK(i2c_master_transmit(
        *sensor_handle, 
        &get_measurement_command, 
        sizeof(get_measurement_command), 
        I2C_TIMEOUT_PRESSURE_SENSOR
    ));
    
    // Receive calibration data
    ESP_ERROR_CHECK(i2c_master_receive(
        *sensor_handle, 
        buffer, 
        sizeof(buffer), 
        I2C_TIMEOUT_PRESSURE_SENSOR
    ));

    // Parse calibration data
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

// Function to calculate pressure using temperature and pressure data
double calculate_pressure(int temp, int press)
{
    int var1, var2;
    uint32_t p;

    // Calculate temperature compensation
    var1 = ((((temp >> 3) - ((int)d_calib.T1 << 1))) * ((int)d_calib.T2)) >> 11;
    var2 = (((((temp >> 4) - ((int)d_calib.T1)) * ((temp >> 4) - ((int)d_calib.T1))) >> 12) * ((int)d_calib.T3)) >> 14;
    temp = var1 + var2;

    // Calculate pressure compensation
    var1 = (((int)temp >> 1) - (int)64000);
    var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((int)d_calib.P6);
    var2 = var2 + ((var1 * ((int)d_calib.P5)) << 1);
    var2 = (var2 >> 2) + (((int)d_calib.P4) << 16);
    var1 = (((d_calib.P3 * ((var1 >> 2) * (var1 >> 2)) >> 13) >> 3) + (((int)d_calib.P2 * var1) >> 1)) >> 18;
    var1 = (((32768 + var1) * ((int)d_calib.P1)) >> 15);

    // Prevent division by zero
    if (var1 == 0) {
        p = 0;
    } else {
        p = (((uint32_t)((int)1048576 - press) - (var2 >> 12)) * 3125);
        p = (p < 0x80000000) ? (p << 1) / (uint32_t)var1 : (p / (uint32_t)var1) * 2;

        // Apply further pressure compensation
        var1 = (((int)d_calib.P9 * ((int)((p >> 3) * (p >> 3)) >> 13)) >> 12);
        var2 = (((int)(p >> 2)) * ((int)d_calib.P8)) >> 13;
        p = (uint32_t)((int)p + ((var1 + var2 + d_calib.P7) >> 4));
    }

    // Return pressure in hPa
    return ((double)p / 100);
}

// Function to start the pressure measurement process
void start_measurement(i2c_master_dev_handle_t *sensor_handle)
{
    const uint8_t start_measurement_command[] = {
        0xF4,  // Control register
        0xB5   // Start measurement
    };

    // Send the command to start the measurement
    ESP_ERROR_CHECK(i2c_master_transmit(
        *sensor_handle, 
        start_measurement_command, 
        sizeof(start_measurement_command), 
        I2C_TIMEOUT_PRESSURE_SENSOR
    ));
}

// Function to retrieve measurement data from the sensor
double get_measurement(i2c_master_dev_handle_t *sensor_handle)
{
    uint8_t measurement_bin[6];
    const uint8_t get_measurement_command = 0xF7;

    // Send the command to read measurement data
    ESP_ERROR_CHECK(i2c_master_transmit(
        *sensor_handle, 
        &get_measurement_command, 
        sizeof(get_measurement_command), 
        I2C_TIMEOUT_PRESSURE_SENSOR
    ));

    // Receive the measurement data
    ESP_ERROR_CHECK(i2c_master_receive(
        *sensor_handle, 
        measurement_bin, 
        sizeof(measurement_bin), 
        I2C_TIMEOUT_PRESSURE_SENSOR
    ));

    // Extract pressure and temperature from the measurement data
    int pressure_bin = (measurement_bin[0] << 12) | (measurement_bin[1] << 4) | (measurement_bin[2] >> 4);
    int temperature_bin = (measurement_bin[3] << 12) | (measurement_bin[4] << 4) | (measurement_bin[5] >> 4);

    // Calculate and return the pressure
    return calculate_pressure(temperature_bin, pressure_bin);
}

// High-level function to read the pressure from the sensor
void read_pressure_sensor(i2c_master_dev_handle_t *sensor_handle, double *pressure)
{
    // Start the measurement process
    start_measurement(sensor_handle);

    // Wait for the measurement to complete
    vTaskDelay(PRESSURE_MEASUREMENT_DURATION);

    // Retrieve and store the pressure measurement
    *pressure = get_measurement(sensor_handle);
}
