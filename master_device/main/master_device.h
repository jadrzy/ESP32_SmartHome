#ifndef MASTER_DEVICE_HEADER
#define MASTER_DEVICE_HEADER

#include <stdint.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"


void init_nvs(uint64_t *);

#endif
