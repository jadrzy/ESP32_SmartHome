#ifndef DATA_HEADER
#define DATA_HEADER

#include "include/constants.h"
#include "include/components.h"
#include "include/nvs.h"
#include "include/wifi.h"

#include <stdint.h>
#include <string.h>
#include <stdbool.h>


typedef struct {
    char serial_number[SERIAL_NUMBER_SIZE];
    uint8_t mac_address[6];
} master_device_t;


typedef struct {
    // ID SECTION
    bool active;
    char serial_number[SERIAL_NUMBER_SIZE];
    uint8_t mac_address[6];
    // DATA SECTION
    bool new_data;
    uint64_t data_buffer;
} slave_device_t;



void get_master_device(char serial[SERIAL_NUMBER_SIZE], uint8_t mac[6]);
void set_master_device(const char serial[SERIAL_NUMBER_SIZE], const uint8_t mac[6]);

void get_slave_devices(slave_device_t devices[NUMBER_OF_DEVICES]);
void set_slave_devices(slave_device_t devices[NUMBER_OF_DEVICES]);

#endif 
