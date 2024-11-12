#include "tasks.h"

// Static handles for I2C and task management
static struct i2c i2c_handles;
static struct task_handles task_handles;

// Mutex handles for sensor data protection
static SemaphoreHandle_t 
    xMutex_Lux,
    xMutex_Humidity,
    xMutex_Temperature,
    xMutex_Pressure;

// Function to initialize all sensors
void initialize_sensors(void)
{
    // Initialize I2C and configure sensors
    i2c_init(&i2c_handles);
    light_sensor_config(&i2c_handles.light_sensor_handle);
    temp_hum_sensor_config(&i2c_handles.temp_hum_sensor_handle);
    pressure_sensor_config(&i2c_handles.pressure_sensor_handle);
}

// Task function to read temperature and humidity values
void task_fun_get_temp_hum_values(void *p)
{
    double humidity_value, temperature_value;

    while (1)
    {
        // Read temperature and humidity sensor data
        read_temp_hum_sensor(&i2c_handles.temp_hum_sensor_handle, &humidity_value, &temperature_value);

        // Protect and update humidity value with mutex
        if (xSemaphoreTake(xMutex_Humidity, 10) == pdTRUE)
        {
            set_humidity(humidity_value);
            xSemaphoreGive(xMutex_Humidity);
        }

        // Protect and update temperature value with mutex
        if (xSemaphoreTake(xMutex_Temperature, 10) == pdTRUE)
        {
            set_temperature(temperature_value);
            xSemaphoreGive(xMutex_Temperature);
        }

        // Delay for the next measurement
        vTaskDelay(TEMP_HUM_SENSOR_MEASUREMENT_TIME);
    }
}

// Task function to read light sensor (lux) values
void task_fun_get_lux_value(void *p)
{
    double lux_value;

    while (1)
    {
        // Read light sensor data
        read_light_sensor(&i2c_handles.light_sensor_handle, &lux_value);

        // Protect and update lux value with mutex
        if (xSemaphoreTake(xMutex_Lux, 10) == pdTRUE)
        {
            set_lux(lux_value);
            xSemaphoreGive(xMutex_Lux);
        }

        // Delay for the next measurement
        vTaskDelay(LIGHT_SENSOR_MEASUREMENT_TIME);
    }
}

// Task function to read pressure values
void task_fun_get_pressure_value(void *p)
{
    double pressure_value;

    while (1)
    {
        // Read pressure sensor data
        read_pressure_sensor(&i2c_handles.pressure_sensor_handle, &pressure_value);

        // Protect and update pressure value with mutex
        if (xSemaphoreTake(xMutex_Pressure, 10) == pdTRUE)
        {
            set_pressure(pressure_value);
            xSemaphoreGive(xMutex_Pressure);
        }

        // Delay for the next measurement
        vTaskDelay(PRESSURE_SENSOR_MEASUREMENT_TIME);
    }
}

// Debug task to log sensor data
void task_debug(void *p)
{
    struct sensor_data_t sensor_data = {0};

    while (1)
    {
        // Read and log lux value
        if (xSemaphoreTake(xMutex_Lux, 10) == pdTRUE)
        {
            sensor_data.lux = get_lux();
            xSemaphoreGive(xMutex_Lux);
        }

        // Read and log humidity value
        if (xSemaphoreTake(xMutex_Humidity, 10) == pdTRUE)
        {
            sensor_data.humidity = get_humidity();
            xSemaphoreGive(xMutex_Humidity);
        }

        // Read and log temperature value
        if (xSemaphoreTake(xMutex_Temperature, 10) == pdTRUE)
        {
            sensor_data.temperature = get_temperature();
            xSemaphoreGive(xMutex_Temperature);
        }

        // Read and log pressure value
        if (xSemaphoreTake(xMutex_Pressure, 10) == pdTRUE)
        {
            sensor_data.pressure = get_pressure();
            xSemaphoreGive(xMutex_Pressure);
        }

        // Log all sensor data
        ESP_LOGI("SENSOR_DATA", "Lux: %g, Humidity: %g, Temperature: %g, Pressure: %g", 
                 sensor_data.lux, sensor_data.humidity, sensor_data.temperature, sensor_data.pressure);

        // Delay for logging
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

// Function to initialize and create tasks
void initialize_tasks(void)
{
    // Create mutexes for sensor data protection
    xMutex_Lux = xSemaphoreCreateMutex();    
    xMutex_Humidity = xSemaphoreCreateMutex();
    xMutex_Temperature = xSemaphoreCreateMutex();
    xMutex_Pressure = xSemaphoreCreateMutex();

    // Create tasks for reading sensors and logging data
    xTaskCreate(task_fun_get_lux_value, "Get_Lux_Task", 6144, NULL, 5, &task_handles.lux_task);
    xTaskCreate(task_fun_get_temp_hum_values, "Get_Temp_Hum_Task", 6144, NULL, 5, &task_handles.temp_hum_task);
    xTaskCreate(task_fun_get_pressure_value, "Get_Pressure_Task", 6144, NULL, 5, &task_handles.press_task);
    xTaskCreate(task_debug, "Debug_Task", 4096, NULL, 5, NULL);
}
