#include "include/ntp.h"
#include "esp_err.h"
#include "include/wifi.h"
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "lwip/arch.h"
#include "lwip/inet.h"
#include "nvs_flash.h"
#include "esp_sntp.h"
#include "esp_netif_types.h"
#include "esp_netif.h"
#include "esp_netif_sntp.h"

static const char* TAG_NTP = "NTP";

wifi_flags_t *flags;

void time_sync_cb(struct timeval *tv)
{
    ESP_LOGI(TAG_NTP, "Time synchronized");
}


void my_sntp_init(void)
{
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    flags = get_wifi_flags();
    if (flags->sntp_initialized == 0)
    {
        ESP_LOGI(TAG_NTP, "Initializing SNTP...");
        config.sync_cb = time_sync_cb;
        esp_netif_sntp_init(&config);
        flags->sntp_initialized = 1;
    }
}

void my_sntp_deinit(void)
{
    flags = get_wifi_flags();
    if (flags->sntp_initialized == 1)
    {
        ESP_LOGI(TAG_NTP, "Deinitializing SNTP...");
        esp_netif_sntp_deinit();
        flags->sntp_initialized = 0;
    }
}
