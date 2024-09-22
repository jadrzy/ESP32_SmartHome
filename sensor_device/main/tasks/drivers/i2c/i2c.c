#include "i2c.h"

// I2C master configuration structure
static const i2c_master_bus_config_t i2c_master_config = {
    .clk_source = I2C_CLK_SRC_DEFAULT,  // Clock source
    .i2c_port = -1,  // Default I2C port 
    .scl_io_num = GPIO_NUM_22,  // SCL pin (clock)
    .sda_io_num = GPIO_NUM_21,  // SDA pin (data)
    .glitch_ignore_cnt = 7,     // Number of glitch pulses to ignore
    .flags.enable_internal_pullup = false,  // Disable internal pull-up resistors
};

// I2C device configuration for slave mode
static i2c_device_config_t i2c_slave_config = {
    .dev_addr_length = I2C_ADDR_BIT_7,  // Device address length (7-bit)
    .scl_speed_hz = 100000,  // I2C bus speed (100 kHz)
};

// Function to initialize the I2C bus
void master_init(i2c_master_bus_handle_t * bus)
{
    // Initialize the I2C master bus with the provided configuration
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_master_config, bus));
}

// Function to initialize an I2C slave device
void slave_init(i2c_master_bus_handle_t * bus, i2c_master_dev_handle_t * sensor, uint8_t address)
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
