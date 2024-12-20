#include "include/data.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "include/task.h"
#include "include/wifi.h"
#include <stdint.h>
#include <string.h>

static const char TAG_DATA[] = "DATA";



// MASTER DEVICE
static master_device_t master_device;

// SLAVE DEVICES
static slave_device_t slave_devices[NUMBER_OF_DEVICES];


void set_master_device(const char serial[SERIAL_NUMBER_SIZE], const uint8_t mac[6])
{
    strcpy(master_device.serial_number, serial);
    memcpy(master_device.mac_address, mac, 6);
}

void set_slave_devices(slave_device_t devices[NUMBER_OF_DEVICES])
{
    for (int i = 0; i < NUMBER_OF_DEVICES; i++)
    {
        slave_devices[i] =  devices[i];
    }
}


esp_err_t set_slave_device_sensor(const uint8_t mac_address[6], const char serial[SERIAL_NUMBER_SIZE], sensor_data_t data)
{
    esp_err_t err = ESP_OK;
    for(int i = 0; i < NUMBER_OF_DEVICES; i++)
    {
        if(slave_devices[i].active)
        {
            if (!strcmp(serial, slave_devices[i].serial_number)) 
            {
                // check if MAC is correct
                bool diff = false;
                for (int j = 0; j < 6; j++)
                {
                    if (mac_address[j] != slave_devices[i].mac_address[j])
                    {
                        diff = true; 
                        err = ESP_ERR_NOT_FOUND;
                        ESP_LOGI(TAG_DATA, "Data is corrupted!");
                    }
                }

                if (!diff)
                {
                    semaphore_handles_t *sem = NULL;
                    sem = get_semaphores();
                    if (xSemaphoreTake(sem->xMutex_sensor_data, 10) == pdTRUE)
                    {
                        slave_devices[i].sensor_data = data;
                        slave_devices[i].new_data = true;
                        xSemaphoreGive(sem->xMutex_sensor_data);
                    }

                }
                return err;
            }
        }
    }
    err = ESP_ERR_NOT_FOUND;
    ESP_LOGI(TAG_DATA, "Device %s not found", serial);
    return err;
}
 

void get_master_device(char serial[SERIAL_NUMBER_SIZE], uint8_t mac[6])
{
    strcpy(serial, master_device.serial_number);
    memcpy(mac, master_device.mac_address, 6);
}

esp_err_t set_light_data(int id, char serial[SERIAL_NUMBER_SIZE], light_control_t data)
{
    esp_err_t err = ESP_ERR_NOT_FOUND;
    semaphore_handles_t *sem = NULL;
    sem = get_semaphores();

    if (strcmp(serial, slave_devices[id].serial_number) == 0)
    {
        if (xSemaphoreTake(sem->xMutex_light_control, 10) == pdTRUE)
        {
            slave_devices[id].light_control = data;
            xSemaphoreGive(sem->xMutex_light_control);
        }
        err = ESP_OK;
    }

    return err;
}

void get_slave_devices(slave_device_t devices[NUMBER_OF_DEVICES])
{
    for (int i = 0; i < NUMBER_OF_DEVICES; i++)
    {
        devices[i] = slave_devices[i];
    }
}

void set_old_data(void)
{
    for (int i = 0; i < NUMBER_OF_DEVICES; i++)
    {
        slave_devices[i].new_data = false;
    }
}
