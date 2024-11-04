#include "include/data.h"
#include "esp_log.h"
#include "include/wifi.h"

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

void get_master_device(char serial[SERIAL_NUMBER_SIZE], uint8_t mac[6])
{
    strcpy(serial, master_device.serial_number);
    memcpy(mac, master_device.mac_address, 6);
    ESP_LOGI(TAG_DATA, "DEBUG");
}

void get_slave_devices(slave_device_t devices[NUMBER_OF_DEVICES])
{
    for (int i = 0; i < NUMBER_OF_DEVICES; i++)
    {
        devices[i] = slave_devices[i];
    }
}
