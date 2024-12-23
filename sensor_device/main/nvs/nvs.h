#ifndef PARTITIONS_HEADER
#define PARTITIONS_HEADER

#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "nvs_flash.h"

#define SERIAL_NUMBER_SIZE 13
#define NUMBER_OF_DEVICES 10

esp_err_t init_nvs_partitions(void);
esp_err_t get_slave_serial_number_from_nvs(char *number);

#endif
