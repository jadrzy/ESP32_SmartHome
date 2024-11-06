#ifndef TASK_HEADER
#define TASK_HEADER

#include "include/constants.h"
#include "include/components.h"
#include "include/data.h"
#include "include/nvs.h"
#include "include/wifi.h"


#include <freertos/queue.h>
#include <freertos/projdefs.h>
#include <freertos/task.h>
#include <freertos/FreeRTOSConfig.h>
#include <freertos/FreeRTOS.h>
#include <freertos/idf_additions.h>
#include <freertos/semphr.h>
#include <stdint.h>
#include <esp_log.h>

typedef struct {
    SemaphoreHandle_t xMutex_sensor_data;
    SemaphoreHandle_t xMutex_light_control;
} semaphore_handles_t;

QueueHandle_t get_rcv_data_handle(void);

void get_semaphores(semaphore_handles_t * sem);
esp_err_t recv_queue_task_init(void);

#endif
