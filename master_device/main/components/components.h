#ifndef COMPONENTS_HEADER
#define COMPONENTS_HEADER

#include "esp_err.h"
#include "components/nvs/nvs.h"
#include "components/wifi/wifi.h"
#include "components/data/data.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_wifi_netif.h"
#include "esp_wifi_types_generic.h"
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include "nvs/nvs.h"
#include <stdint.h>

// Constants
#define SERIAL_NUMBER_SIZE 13
#define NUMBER_OF_DEVICES 10
#define WIFI_CREDENTIALS_SIZE 33 

typedef struct {
    char serial[SERIAL_NUMBER_SIZE];
    uint64_t mac;
} device_t;

esp_err_t memory_setup(void);
esp_err_t memory_update(void);
void mac_8_64(const uint8_t input[6], uint64_t *output);
void mac_64_8(const uint64_t input, uint8_t output[6]);


#endif
