#include "include/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "include/data.h"
#include "include/led.h"
#include "include/wifi.h"
#include "extlib/cJSON.h"
#include <stdio.h>
#include <string.h>

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
    ESP_LOGI(TAG_TASK, "Recieved data: lux = %g | pressure = %g | humidity = %g | temperature = %g", 
             data->lux, data->pressure, data->humidity, data->temperature);
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
            handle_recv_queue_data(&recv_data);
        }
    }
}

void send_data_task(void *p)
{
    static slave_device_t devices[NUMBER_OF_DEVICES];
    static send_data_t data;
    static wifi_flags_t *flags;
    flags = get_wifi_flags();

    while(1)
    {
        if (flags->sta_connected)
        {
            get_slave_devices(devices);

            for (int i = 0; i < NUMBER_OF_DEVICES; i++)
            {
                if (devices[i].active)
                {
                    strcpy(data.serial, devices[i].serial_number);
                    memcpy(data.mac_address, devices[i].mac_address, sizeof(data.mac_address));
                    if (xSemaphoreTake(semaphores.xMutex_light_control, 10) == pdTRUE) 
                    {
                        data.auto_light = devices[i].light_control.auto_light;
                        data.light_value = devices[i].light_control.light_value;
                        xSemaphoreGive(semaphores.xMutex_light_control);
                    }
                    ESP_LOGI(TAG_TASK, "Sending data: lm = %d | light_value = %d", data.auto_light, data.light_value);
                    send_espnow_data(data);
                }
            }
        }
        vTaskDelay(DATA_REQUEST_PERIOD);
    }
}


void wifi_reboot_and_led_toggle_task(void *p)
{
    wifi_flags_t *flags;
    flags = get_wifi_flags();

    while(1)
    {
        // REBOOT CONTROL
        if (flags->reboot == 1)     
        {
            wifi_reboot();
            flags->reboot = 0;
        }

        // LED CONTROL
        if (flags->setup_mode)
        {
            toggle_wifi_led();
        }
        else if (flags->sta_connected) 
        {
            set_on_wifi_led();
        }
        vTaskDelay(WIFI_REBOOT_CHECK_TIME);
    }
}


static char *json_maker(master_device_t master_device, slave_device_t devices[NUMBER_OF_DEVICES])
{
    char *string = NULL;
    cJSON *serial_master;
    cJSON *timestamp;
    cJSON *serial_slave;
    cJSON *data_slave;
    cJSON *lux;
    cJSON *temperature;
    cJSON *humidity;
    cJSON *pressure;
    char key[16];
    char tail[2];

    // Creating JSON
    cJSON *data = cJSON_CreateObject();
    if (data == NULL)
    {
        goto end;
    }

    // Adding to JSON master serial number 
    serial_master = cJSON_CreateString(master_device.serial_number);
    if (serial_master == NULL)
    {
        goto end;
    }
    cJSON_AddItemToObject(data, "serial_master", serial_master);

    // Adding to JSON current timestamp
    time_t now = 0;
    time(&now);
    timestamp = cJSON_CreateNumber(now);
    if (timestamp == NULL)
    {
        goto end;
    }
    cJSON_AddItemToObject(data, "timestamp", timestamp);

    // Adding to JSON active slave devices and new data
    for (int i = 0; i < NUMBER_OF_DEVICES; i++)
    {
        // Slave device serial number
        if (devices[i].active)
        {
            sprintf(tail, "%d", i);
            strcpy(key, "serial_slave_");
            strcat(key, tail);
            serial_slave = cJSON_CreateString(devices[i].serial_number);
            if (serial_slave == NULL)
            {
                goto end;
            }
            cJSON_AddItemToObject(data, key, serial_slave);

            // New data
            if (devices[i].new_data == true)
            {
                strcpy(key, "data_slave_");
                strcat(key, tail);
                data_slave = cJSON_CreateObject();
                if (data_slave == NULL)
                {
                    goto end;
                }
                cJSON_AddItemToObject(data, key, data_slave);

                // Lux
                lux = cJSON_CreateNumber(devices[i].sensor_data.lux);
                if (lux == NULL)
                {
                    goto end;
                }
                cJSON_AddItemToObject(data_slave, "lux", lux);

                // Temperature
                temperature = cJSON_CreateNumber(devices[i].sensor_data.temperature);
                if (temperature == NULL)
                {
                    goto end;
                }
                cJSON_AddItemToObject(data_slave, "temperature", temperature);

                // Humidity
                humidity = cJSON_CreateNumber(devices[i].sensor_data.humidity);
                if (humidity == NULL)
                {
                    goto end;
                }
                cJSON_AddItemToObject(data_slave, "humidity", humidity);

                // Pressure
                pressure = cJSON_CreateNumber(devices[i].sensor_data.pressure);
                if (pressure == NULL)
                {
                    goto end;
                }
                cJSON_AddItemToObject(data_slave, "pressure", pressure);
            }
        }

        string = cJSON_Print(data);
        if (string == NULL)
        {
            ESP_LOGI(TAG_TASK, "Failed to produce JSON");
        }
    }

    end:
        cJSON_Delete(data);
        return string;
}


void wifi_send_to_db_task(void *p)
{
    static master_device_t master_device;
    static slave_device_t devices[NUMBER_OF_DEVICES];
    static slave_device_t devices_clone[NUMBER_OF_DEVICES];
    get_slave_devices(devices);
    get_master_device(master_device.serial_number, master_device.mac_address);

    static wifi_flags_t *flags;
    flags = get_wifi_flags();
    char * data_str;
    
    while(1)
    {
        if (flags->got_ip && flags->time_synchronized)
        {
            if (xSemaphoreTake(semaphores.xMutex_sensor_data, 10) == pdTRUE) 
            {
                memcpy(devices_clone, devices, sizeof(devices)); 
                xSemaphoreGive(semaphores.xMutex_sensor_data);
            }

            data_str = json_maker(master_device, devices_clone); 

        }
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

esp_err_t recv_queue_task_init(void)
{
    esp_err_t err = ESP_OK;

    semaphores.xMutex_sensor_data = xSemaphoreCreateMutex();    
    semaphores.xMutex_light_control = xSemaphoreCreateMutex();
    
    recieve_data_queue = xQueueCreate(NUMBER_OF_DEVICES, sizeof(recv_data_t));

    xTaskCreatePinnedToCore(recv_queue_task, "Recieve queue task", 8192, NULL, 5, &task_handles.recv_queue, 1);
    xTaskCreatePinnedToCore(send_data_task, "Send data task", 8192, NULL, 4, &task_handles.send_data, 1);
    xTaskCreatePinnedToCore(wifi_reboot_and_led_toggle_task, "Wifi reboot and led toggle task", 8192, NULL, 3, &task_handles.reboot_and_led, 1);
    xTaskCreatePinnedToCore(wifi_send_to_db_task, "Task responsible for sending data to database", 8192, NULL, 5, &task_handles.send_to_db, 1);
    return err;
}



