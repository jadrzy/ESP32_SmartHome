#include "include/task.h"
#include "esp_err.h"
#include "freertos/idf_additions.h"
#include "include/data.h"
#include "include/wifi.h"

#define QUEUE_TASK_WAIT_TIME (10 / portTICK_PERIOD_MS)

static const char TAG_TASK[] = "TASK";

static QueueHandle_t recieve_data_queue;

QueueHandle_t get_rcv_data_handle(void) {return recieve_data_queue;}

static semaphore_handles_t semaphores;


void get_semaphores(semaphore_handles_t * sem)
{
    sem = &semaphores;
}

static esp_err_t handle_recv_queue_data(recieve_data_t * data)
{
    esp_err_t err = ESP_OK;
    err = set_slave_device_sensor(data->mac_address, data->serial_number, data->sensor_data);
    return err;
}


static void recv_queue_task(void *p)
{
    static recieve_data_t recv_data;

    ESP_LOGI(TAG_TASK, "Recieve queue task is working...");

    while (1)
    {
        if(xQueueReceive(recieve_data_queue, &recv_data, portMAX_DELAY) == pdTRUE)
        {
            handle_recv_queue_data(&recv_data);
        }
        vTaskDelay(QUEUE_TASK_WAIT_TIME);
    }

}



esp_err_t recv_queue_task_init(void)
{
    semaphores.xMutex_sensor_data = xSemaphoreCreateMutex();    
    semaphores.xMutex_light_control = xSemaphoreCreateMutex();

    esp_err_t err = ESP_OK;
    recieve_data_queue = xQueueCreate(NUMBER_OF_DEVICES, sizeof(recieve_data_t));

    err = xTaskCreate(recv_queue_task, "recv_task", 8192, NULL, 4, NULL);

    return err;
}



