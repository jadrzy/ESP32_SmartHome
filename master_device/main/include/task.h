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

#define WIFI_REBOOT_CHECK_TIME (1000 / portTICK_PERIOD_MS)

typedef struct {
    SemaphoreHandle_t xMutex_sensor_data;
    SemaphoreHandle_t xMutex_light_control;
} semaphore_handles_t;

typedef struct {
    TaskHandle_t recv_queue;
    TaskHandle_t send_data;
    TaskHandle_t reboot;
} task_handles_t;

QueueHandle_t get_rcv_data_handle(void);

esp_err_t recv_queue_task_init(void);
semaphore_handles_t* get_semaphores(void);

#endif
