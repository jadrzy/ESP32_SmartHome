#include "i2c.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "i2c/light_sensor/light_sensor.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

void i2c_init(void)
{
    bus_handle = master_bus_init();
    light_sensor_handle = slave_dev_init( LIGHT_SENSOR_ADDRESS, bus_handle);
}

void sensor_config(void)
{
    light_sensor_config(light_sensor_handle); 
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    ESP_LOGI("READ", "Read value: %f", read_light_sensor(light_sensor_handle));
}


i2c_master_bus_handle_t master_bus_init(void)
{
    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = -1,
        .scl_io_num = GPIO_NUM_22,
        .sda_io_num = GPIO_NUM_21,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = false,
    };
    
    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK( i2c_new_master_bus(&i2c_mst_config, &bus_handle) );

    return bus_handle;
}

i2c_master_dev_handle_t slave_dev_init(uint16_t address, i2c_master_bus_handle_t master_bus_handle)
{
    i2c_device_config_t cfg = {
        .dev_addr_length = I2C_ADDR_BIT_7,
        .scl_speed_hz = 100000,
        .device_address = address,
    };

    i2c_master_dev_handle_t dev_handle;
    ESP_ERROR_CHECK( i2c_master_bus_add_device(master_bus_handle, &cfg, &dev_handle) );

    return dev_handle;
}
