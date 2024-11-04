#ifndef COMPONENTS_HEADER
#define COMPONENTS_HEADER

#include "include/constants.h"
#include "include/data.h"
#include "include/nvs.h"
#include "include/wifi.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_wifi_netif.h"
#include "esp_wifi_types_generic.h"
#include <stdint.h>
#include <string.h>
#include <inttypes.h>


esp_err_t memory_setup(void);
esp_err_t memory_update(void);
void mac_8_64(const uint8_t input[6], uint64_t *output);
void mac_64_8(const uint64_t input, uint8_t output[6]);


#endif
