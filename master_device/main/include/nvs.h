#ifndef PARTITIONS_HEADER
#define PARTITIONS_HEADER


#include "include/constants.h"
#include "include/components.h"
#include "include/data.h"
#include "include/wifi.h"

#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "nvs_flash.h"
#include "esp_partition.h"


// Function declarations
esp_err_t init_nvs_partitions(void);
esp_err_t get_master_serial_number_from_nvs(char *number);
esp_err_t get_wifi_sm_cred_from_nvs(char * SSID, char * PSSWD);
esp_err_t write_wifi_sm_cred_to_nvs(char * SSID,char * PSSWD);
esp_err_t get_paired_devices_from_nvs(uint64_t mac_device_list[NUMBER_OF_DEVICES], char serial_device_list[NUMBER_OF_DEVICES][SERIAL_NUMBER_SIZE]);
esp_err_t write_paired_devices_to_nvs(uint64_t mac_device_list[NUMBER_OF_DEVICES], char serial_device_list[NUMBER_OF_DEVICES][SERIAL_NUMBER_SIZE]);

#endif // PARTITIONS_HEADER
