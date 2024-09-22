#include "tasks.h"

static struct i2c i2c_handles;
static struct task_handles task_handles;
static SemaphoreHandle_t xMutex_Lux; 

void initialize_sensors(void)
{
    i2c_init(&i2c_handles);
    light_sensor_config(&i2c_handles.light_sensor_handle);
}

void task_fun_get_lux_value(void *p)
{
    while(1)
    {
        uint32_t temp_lux;
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
    while(1)
    {
        if( xSemaphoreTake(xMutex_Lux, 10) == pdTRUE)
        {
            ESP_LOGI("TAG", "Data:\t Lux = %d", (int) get_lux());
            xSemaphoreGive(xMutex_Lux);
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void initialize_tasks(void)
{
    xMutex_Lux = xSemaphoreCreateMutex();    

    xTaskCreate(
        task_fun_get_lux_value, 
        "GET LUX TASK", 
        4096, 
        NULL, 
        5, 
        &task_handles.lux_task
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
