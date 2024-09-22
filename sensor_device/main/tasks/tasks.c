#include "tasks.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "tasks/data/data.h"
#include <stdint.h>

static struct i2c i2c_handles;
static struct task_handles task_handles;
static SemaphoreHandle_t 
    xMutex_Lux,
    xMutex_Humidity,
    xMutex_Temperature,
    xMutex_Pressure;

void initialize_sensors(void)
{
    i2c_init(&i2c_handles);
    light_sensor_config(&i2c_handles.light_sensor_handle);
    temp_hum_sensor_config(&i2c_handles.temp_hum_sensor_handle);
}

void task_fun_get_temp_hum_values(void *p)
{
    double 
        temp_hum, 
        temp_temp;

    while(1)
    {
        read_temp_hum_sensor(&i2c_handles.temp_hum_sensor_handle, &temp_hum, &temp_temp);

        if( xSemaphoreTake(xMutex_Humidity, 10) == pdTRUE)
        {
            set_humidity(temp_hum);
            xSemaphoreGive(xMutex_Humidity);
        }

        if( xSemaphoreTake(xMutex_Temperature, 10) == pdTRUE)
        {
            set_temperature(temp_temp);
            xSemaphoreGive(xMutex_Temperature);
        }

        vTaskDelay(TEMP_HUM_SENSOR_MEASUREMENT_TIME);
    }
}


void task_fun_get_lux_value(void *p)
{
    double temp_lux;
    while(1)
    {
        read_light_sensor(&i2c_handles.light_sensor_handle, &temp_lux);

        if( xSemaphoreTake(xMutex_Lux, 10) == pdTRUE)
        {
            set_lux(temp_lux);
            xSemaphoreGive(xMutex_Lux);
        }
        vTaskDelay(LIGHT_SENSOR_MEASUREMENT_TIME);
    }
}


void task_debug(void *p)
{
    struct data temp_data = {0};
    while(1)
    {
        if( xSemaphoreTake(xMutex_Lux, 10) == pdTRUE)
        {
            temp_data.lux = get_lux();
            xSemaphoreGive(xMutex_Lux);
        }
        if( xSemaphoreTake(xMutex_Humidity, 10) == pdTRUE)
        {
            temp_data.humidity = get_humidity();
            xSemaphoreGive(xMutex_Humidity);
        }
        if( xSemaphoreTake(xMutex_Temperature, 10) == pdTRUE)
        {
            temp_data.temperature = get_temperature();
            xSemaphoreGive(xMutex_Temperature);
        }
        if( xSemaphoreTake(xMutex_Pressure, 10) == pdTRUE)
        {
            temp_data.pressure = get_pressure();
            xSemaphoreGive(xMutex_Pressure);
        }

        ESP_LOGI("TAG", "Data:\t Lux = %g\t Humidity = %g\t Temperature = %g\t Pressure = %g", 
                 temp_data.lux, temp_data.humidity, temp_data.temperature, temp_data.pressure);

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void initialize_tasks(void)
{
    xMutex_Lux = xSemaphoreCreateMutex();    
    xMutex_Humidity = xSemaphoreCreateMutex();
    xMutex_Temperature = xSemaphoreCreateMutex();
    xMutex_Pressure = xSemaphoreCreateMutex();

    xTaskCreate(
        task_fun_get_lux_value, 
        "GET LUX TASK", 
        6144, 
        NULL, 
        5, 
        &task_handles.lux_task
    );

    xTaskCreate(
        task_fun_get_temp_hum_values, 
        "GET TEMP HUM TASK", 
        6144, 
        NULL, 
        5, 
        &task_handles.temp_hum_task
    );

    xTaskCreate(
        task_debug, 
        "DEBBUG TASK", 
        4096, 
        NULL, 
        5, 
        NULL 
    );

}
