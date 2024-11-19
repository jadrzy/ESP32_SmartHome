#include "include/ntp.h"
#include "esp_err.h"
#include "freertos/idf_additions.h"
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
static TimerHandle_t sntp_synch_timer = NULL;

void time_sync_cb(struct timeval *tv)
{
    flags = get_wifi_flags();
    if (flags->time_synchronized == 0)
    {
        xTimerStart(sntp_synch_timer, 5);
    }
    else
    {
        xTimerReset(sntp_synch_timer, 5);
    }
    ESP_LOGI(TAG_NTP, "Time synchronized");


    char strftime_buf[64];
    time_t now = 0;
    struct tm timeinfo = { 0 };
    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG_NTP, "The current time in Cracow is: %s", strftime_buf);
    flags->time_synchronized = 1;
}


void callback_synch_time_expired(TimerHandle_t xTimer)
{
    ESP_LOGE(TAG_NTP, "Time is not synchronized for too long.");
    ESP_LOGE(TAG_NTP, "REBOOTING...");
    esp_restart();
}

void my_sntp_init(void)
{
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    flags = get_wifi_flags();
    if (flags->sntp_initialized == 0)
    {
        ESP_LOGI(TAG_NTP, "Initializing SNTP...");

        sntp_synch_timer = xTimerCreate(
            "SNTP synchronization timer", 
            pdMS_TO_TICKS(MAX_SNTP_SYNCH_TIME), 
            pdFALSE, 
            (void*)0, 
            callback_synch_time_expired);

        sntp_set_sync_interval((int)(MAX_SNTP_SYNCH_TIME / 4));

        ESP_LOGI(TAG_NTP, "Time synchronization interval = %d ms", (int) sntp_get_sync_interval());

        config.sync_cb = time_sync_cb;
        esp_netif_sntp_init(&config);
        flags->sntp_initialized = 1;

        setenv("TZ", "CTS-1", 1);
        tzset();
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
        xTimerDelete(sntp_synch_timer, 5);
    }
}
