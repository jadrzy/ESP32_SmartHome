#include "i2c.h"

static void master_init(i2c_master_bus_handle_t * bus)
{
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_master_config, bus));
}


static void slave_init(i2c_master_bus_handle_t * bus, i2c_master_dev_handle_t * sensor, uint8_t address)
{
    i2c_slave_config.device_address = address;  
    ESP_ERROR_CHECK(i2c_master_bus_add_device(*bus, &i2c_slave_config, sensor));
}

void i2c_init(struct i2c * i2c)
{
    master_init(&i2c->bus_handle);
    slave_init(&i2c->bus_handle, &i2c->light_sensor_handle, LIGHT_SENSOR_ADDRESS);
    slave_init(&i2c->bus_handle, &i2c->temp_hum_sensor_handle, TEMP_HUM_SENSOR_ADDRESS);
    slave_init(&i2c->bus_handle, &i2c->pressure_sensor_handle, PRESSURE_SENSOR_ADDRESS);
    i2c->init_status = 1;
}
