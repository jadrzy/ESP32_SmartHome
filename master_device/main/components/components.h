#include "esp_err.h"
#include "nvs/nvs.h"
#include <stdint.h>

#ifndef COMPONENTS_HEADER
#define COMPONENTS_HEADER

// Constants
#define SERIAL_NUMBER_SIZE 13
#define NUMBER_OF_DEVICES 10
#define WIFI_CREDENTIALS_SIZE 33 

typedef struct {
    char ssid[WIFI_CREDENTIALS_SIZE];
    char psswd[WIFI_CREDENTIALS_SIZE];
} wifi_sta_cred_t;

typedef struct {
    char serial[SERIAL_NUMBER_SIZE];
    uint64_t mac;
} device_t;

esp_err_t run(void);

#endif
