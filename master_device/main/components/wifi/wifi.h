#ifndef WIFI_HEADER
#define WIFI_HEADER


#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include "esp_err.h"
#include "esp_netif_types.h"
#include "esp_wifi_default.h"
#include "esp_wifi_types_generic.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "nvs_flash.h"
#include "esp_random.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_now.h"
#include "esp_crc.h"
#include "components/components.h"

esp_err_t wifi_init(bool setup_active);

#endif
