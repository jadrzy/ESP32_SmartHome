#ifndef HEADER_DATA_H
#define HEADER_DATA_H

#include <stdint.h>

struct data {
    uint32_t temperature,
             humidity,
             pressure,
             lux;
};

extern uint32_t get_lux(void);
extern void set_lux(uint32_t);

#endif
