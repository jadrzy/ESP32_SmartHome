#ifndef HEADER_DATA_H
#define HEADER_DATA_H

#include <stdint.h>

struct data {
    double lux;
    double temperature,
           humidity,
           pressure;
};

extern double get_lux(void);
extern double get_temperature(void);
extern double get_humidity(void);
extern double get_pressure(void);

extern void set_lux(double);
extern void set_temperature(double);
extern void set_humidity(double);
extern void set_pressure(double);

#endif
