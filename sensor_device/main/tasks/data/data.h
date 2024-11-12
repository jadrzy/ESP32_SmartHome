#ifndef HEADER_DATA_H
#define HEADER_DATA_H

#include <stdint.h>
#include <stdbool.h>

#define SERIAL_NUMBER_SIZE      13

typedef struct {
    char serial_number[SERIAL_NUMBER_SIZE];
    uint8_t mac_address[6];
} slave_device_t;

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

double get_lux(void);
double get_temperature(void);
double get_humidity(void);
double get_pressure(void);
bool get_light_mode(void);
int get_light_value(void);
void get_slave_device(char serial[SERIAL_NUMBER_SIZE], uint8_t mac[6]);

void set_lux(double);
void set_temperature(double);
void set_humidity(double);
void set_pressure(double);
void set_light_mode(bool);
void set_light_value(int);
void set_slave_device(const char serial[SERIAL_NUMBER_SIZE], const uint8_t mac[6]);

esp_err_t memory_setup(void);


#endif
