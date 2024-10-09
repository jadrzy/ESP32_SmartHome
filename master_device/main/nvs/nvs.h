#ifndef PARTITIONS_HEADER
#define PARTITIONS_HEADER

#include <stdint.h>
#include "esp_err.h"
#include "master_device.h"
#include "nvs_flash.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_log.h"
#include "nvs_flash.h"

void init_nvs_partitions(void);
void get_master_serial_number_from_nvs(char * number);
void get_slave_serial_number_from_nvs(char number[][SERIAL_NUMBER_SIZE + 1]);
void write_slave_serial_number_to_nvs(char number[][SERIAL_NUMBER_SIZE + 1]);
// void operation(void);

#endif
