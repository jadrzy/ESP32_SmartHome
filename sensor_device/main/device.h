#include "drivers/i2c/i2c.h"
#ifndef  HEADER_DEVICE_H
#define  HEADER_DEVICE_H

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdint.h>
#include <stdint.h>

struct data {
    double  temperature,
            humidity,
            pressure,
            lux;
};

#endif
