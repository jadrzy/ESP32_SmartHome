#ifndef  HEADER_DEVICE_H
#define  HEADER_DEVICE_H

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdint.h>

struct Data {
    uint32_t temperature,
             humidity,
             pressure,
             lux;
};

#endif
