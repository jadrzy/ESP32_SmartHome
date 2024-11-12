#include "data/data.h"
#include "drivers/i2c/i2c.h"
#include "drivers/light_sensor/light_sensor.h"
#include "drivers/temp_hum_sensor/temp_hum_sensor.h"
#include "drivers/pressure_sensor/pressure_sensor.h"
#include "tasks/wifi/wifi.h"

#include "freertos/projdefs.h"
#include "freertos/idf_additions.h"
#include "freertos/semphr.h"
#include <stdint.h>

#ifndef HEADER_TASKS_H
#define HEADER_TASKS_H

// Sensor measurement times in ticks
#define LIGHT_SENSOR_MEASUREMENT_TIME     (15 / portTICK_PERIOD_MS)   // 15ms for light sensor measurement
#define TEMP_HUM_SENSOR_MEASUREMENT_TIME  (5 / portTICK_PERIOD_MS)    // 5ms for temperature & humidity sensor (+85ms in measurement)
#define PRESSURE_SENSOR_MEASUREMENT_TIME  (5 / portTICK_PERIOD_MS)    // 5ms for pressure sensor (+45ms in measurement)

// Structure to hold task handles for FreeRTOS tasks
struct task_handles {
    TaskHandle_t lux_task;        // Task handle for light sensor
    TaskHandle_t temp_hum_task;   // Task handle for temperature & humidity sensor
    TaskHandle_t press_task;      // Task handle for pressure sensor
    TaskHandle_t wifi_task;       // Task handle for Wi-Fi task
    TaskHandle_t recieve_data_task;
};


// Function to initialize all sensors
void initialize_sensors(void);

// Task function declarations
void task_fun_get_lux_value(void *pvParameters);          // Task to get light sensor value
void task_fun_get_temp_hum_values(void *pvParameters);    // Task to get temperature & humidity sensor values
void task_fun_get_pressure_value(void *pvParameters);     // Task to get pressure sensor value
void task_debug(void *pvParameters);                      // Debug task function

// Function to initialize and create FreeRTOS tasks
void initialize_tasks(void);

#endif // HEADER_TASKS_H
