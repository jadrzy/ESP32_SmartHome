#include "include/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "include/data.h"
#include "include/wifi.h"

#define QUEUE_TASK_WAIT_TIME (10 / portTICK_PERIOD_MS)

static const char TAG_TASK[] = "TASK";

static QueueHandle_t recieve_data_queue;

static task_handles_t task_handles;
static semaphore_handles_t semaphores;


semaphore_handles_t* get_semaphores(void) {return &semaphores;}
QueueHandle_t get_rcv_data_handle(void) {return recieve_data_queue;}


static esp_err_t handle_recv_queue_data(recv_data_t * data)
{
    esp_err_t err = ESP_OK;
    sensor_data_t s_data = {
        .lux = data->lux,
        .humidity = data->humidity,
        .pressure = data->pressure,
        .temperature = data->temperature
    };

    err = set_slave_device_sensor(data->mac_address, data->serial, s_data);
    return err;
}


void recv_queue_task(void *p)
{
    static recv_data_t recv_data;

    ESP_LOGI(TAG_TASK, "Recieve queue task is working...");

    while (1)
    {
        if(xQueueReceive(recieve_data_queue, &recv_data, portMAX_DELAY) == pdTRUE)
        {
            ESP_LOGI(TAG_TASK, "Data added to the queue");
            handle_recv_queue_data(&recv_data);
        }

    }

}



esp_err_t recv_queue_task_init(void)
{
    esp_err_t err = ESP_OK;

    semaphores.xMutex_sensor_data = xSemaphoreCreateMutex();    
    semaphores.xMutex_light_control = xSemaphoreCreateMutex();
    
    recieve_data_queue = xQueueCreate(NUMBER_OF_DEVICES, sizeof(recv_data_t));

    xTaskCreate(recv_queue_task, "Recieve queue task", 8192, NULL, 5, &task_handles.recv_queue);

    return err;
}



