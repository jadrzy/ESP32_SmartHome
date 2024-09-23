#include "data/data.h"
#include "drivers/i2c/i2c.h"
#include "drivers/light_sensor/light_sensor.h"
#include "drivers/temp_hum_sensor/temp_hum_sensor.h"
#include "drivers/pressure_sensor/pressure_sensor.h"

#include "freertos/idf_additions.h"
#include "freertos/semphr.h"
#include <stdint.h>

#ifndef HEADER_TASKS_H
#define HEADER_TASKS_H

#define LIGHT_SENSOR_MEASUREMENT_TIME (15 / portTICK_PERIOD_MS)
#define TEMP_HUM_SENSOR_MEASUREMENT_TIME (5 / portTICK_PERIOD_MS) // (85ms included during measurement)
#define PRESSURE_SENSOR_MEASUREMENT_TIME (5 / portTICK_PERIOD_MS) // (45ms included during measurement)
//
struct task_handles {
    TaskHandle_t
        lux_task,
        temp_hum_task,
        press_task,
        wifi_task;
};


void initialize_sensors(void);

// Task functions
void task_fun_get_lux_value(void *);
void task_fun_get_temp_hum_values(void *);
void task_fun_get_pressure_value(void *);
void task_debud(void*);

// Create tasks fucntion
void initialize_tasks(void); 

#endif
