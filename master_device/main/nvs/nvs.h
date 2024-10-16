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

// Constants
#define SERIAL_NUMBER_SIZE 12
#define NUMBER_OF_DEVICES 10
#define WIFI_CREDENTIALS_SIZE 32 

// Function declarations
void init_nvs_partitions(void);
void get_master_serial_number_from_nvs(char *number);
void get_wifi_sm_cred_from_nvs(char * SSID, char * PSSWD);
void get_slave_serial_number_from_nvs(char number[][SERIAL_NUMBER_SIZE + 1]);
void write_slave_serial_number_to_nvs(char number[][SERIAL_NUMBER_SIZE + 1]);
void write_wifi_sm_cred_to_nvs(char * SSID,char * PSSWD);

#endif // PARTITIONS_HEADER
