#include "i2c.h"

// Function to initialize the I2C bus
static void master_init(i2c_master_bus_handle_t * bus)
{
    // Initialize the I2C master bus with the provided configuration
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_master_config, bus));
}

// Function to initialize an I2C slave device
static void slave_init(i2c_master_bus_handle_t * bus, i2c_master_dev_handle_t * sensor, uint8_t address)
{
    // Set the device address for the slave
    i2c_slave_config.device_address = address;  
    // Add the slave device to the I2C bus
    ESP_ERROR_CHECK(i2c_master_bus_add_device(*bus, &i2c_slave_config, sensor));
}

// Function to initialize the entire I2C setup (master and slave devices)
void i2c_init(struct i2c * i2c)
{
    // Initialize the I2C bus master
    master_init(&i2c->bus_handle);
    // Initialize the light sensor as an I2C slave device
    slave_init(&i2c->bus_handle, &i2c->light_sensor_handle, LIGHT_SENSOR_ADDRESS);
    // Initialize the temperature and humidity sensor as an I2C slave device
    slave_init(&i2c->bus_handle, &i2c->temp_hum_sensor_handle, TEMP_HUM_SENSOR_ADDRESS);
    // Initialize the pressure sensor as an I2C slave device
    slave_init(&i2c->bus_handle, &i2c->pressure_sensor_handle, PRESSURE_SENSOR_ADDRESS);
}
