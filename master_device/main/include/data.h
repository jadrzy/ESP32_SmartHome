#ifndef DATA_HEADER
#define DATA_HEADER

#include "freertos/idf_additions.h"
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
    double lux;
    double humidity;
    double pressure;
    double temperature; 
} sensor_data_t;


typedef struct {
    bool auto_light;
    int light_value;
} light_control_t;


typedef struct {

    // ID SECTION
    bool active;
    char serial_number[SERIAL_NUMBER_SIZE];
    uint8_t mac_address[6];

    // DATA SECTION
    bool new_data;
    sensor_data_t sensor_data;
    light_control_t light_control;
    
} slave_device_t;



void get_master_device(char serial[SERIAL_NUMBER_SIZE], uint8_t mac[6]);
void set_master_device(const char serial[SERIAL_NUMBER_SIZE], const uint8_t mac[6]);

void get_slave_devices(slave_device_t devices[NUMBER_OF_DEVICES]);
void set_slave_devices(slave_device_t devices[NUMBER_OF_DEVICES]);
void set_old_data(void);

esp_err_t set_slave_device_sensor(const uint8_t mac_address[6], const char serial[SERIAL_NUMBER_SIZE], sensor_data_t data);
esp_err_t set_light_data(int id, char serial[SERIAL_NUMBER_SIZE], light_control_t data);

#endif 
