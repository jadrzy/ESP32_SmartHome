#ifndef WIFI_HEADER
#define WIFI_HEADER

#include "include/constants.h"
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

#define MAX_SETUP_TIME (1 * 60 * 1000)      // 1min

typedef struct {
    unsigned int setup_mode : 1;
    unsigned int wifi_initialized : 1;
    unsigned int esp_now_initiated : 1;
    unsigned int rtc_synchronized : 1;
    unsigned int sta_connected : 1;
    unsigned int ap_connected : 1;
    unsigned int reboot : 1;
} wifi_flags_t;

typedef struct {
    uint8_t channel;
    uint8_t mac_address[6];
    char serial[SERIAL_NUMBER_SIZE];
    bool auto_light;
    int light_value;
} send_data_t;

typedef struct {
    uint8_t mac_address[6];
    char serial[SERIAL_NUMBER_SIZE];
    double lux;
    double humidity;
    double pressure;
    double temperature; 
} recv_data_t;


esp_err_t wifi_init();
esp_err_t my_esp_now_init(void);
esp_err_t send_espnow_data(send_data_t data);
wifi_flags_t* get_wifi_flags(void);
esp_err_t start_setup_mode(void);
esp_err_t stop_setup_mode(void);
void wifi_reboot(void);


#endif
