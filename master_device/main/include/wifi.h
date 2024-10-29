#ifndef WIFI_HEADER
#define WIFI_HEADER

#include "include/components.h"
#include "include/data.h"
#include "include/nvs.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_wifi_default.h"
#include "esp_wifi_types_generic.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "esp_event_base.h"
#include "esp_event.h"
#include "esp_random.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_now.h"
#include "esp_crc.h"
#include "esp_netif_net_stack.h"
#include "esp_netif_types.h"
#include "esp_netif.h"
#include "lwip/def.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "lwip/lwip_napt.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "nvs_flash.h"


typedef struct {
    unsigned int setup_mode : 1;
    unsigned int wifi_initialized : 1;
    unsigned int esp_now_initiated : 1;
    unsigned int sta_connected : 1;
    unsigned int ap_connected : 1;
} wifi_flags_t;


esp_err_t wifi_init();
esp_err_t my_esp_now_init(void);

void wifi_reboot(void);

#endif
