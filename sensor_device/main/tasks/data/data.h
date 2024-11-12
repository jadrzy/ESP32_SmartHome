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

extern double get_lux(void);
extern double get_temperature(void);
extern double get_humidity(void);
extern double get_pressure(void);

extern void set_lux(double);
extern void set_temperature(double);
extern void set_humidity(double);
extern void set_pressure(double);

#endif
