#include "components/components.h"
#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "nvs_flash.h"
#include "esp_partition.h"
#include "master_device.h"
#include "nvs_flash.h"

#ifndef PARTITIONS_HEADER
#define PARTITIONS_HEADER


// Function declarations
esp_err_t init_nvs_partitions(void);
esp_err_t get_master_serial_number_from_nvs(char *number);
esp_err_t get_wifi_sm_cred_from_nvs(char * SSID, char * PSSWD);
esp_err_t write_wifi_sm_cred_to_nvs(char * SSID,char * PSSWD);
esp_err_t get_paired_devices_from_nvs(device_t device_list[NUMBER_OF_DEVICES]);
esp_err_t write_paired_devices_to_nvs(device_t device_list[NUMBER_OF_DEVICES]);

#endif // PARTITIONS_HEADER
