#include "include/ntp.h"
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
#include "nvs_flash.h"
#include "esp_sntp.h"


static const char* TAG_NTP = "NTP";

void time_sync_cb(struct timeval *tv)
{
    wifi_flags_t *flags = get_wifi_flags();
    ESP_LOGI(TAG_NTP, "Time synchronized");
    flags->rtc_synchronized = 1;
}

void synch_time(void)
{
    ESP_LOGI(TAG_NTP, "Initializing SNTP");

    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_cb);
    sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);

    esp_sntp_init();

    // Opcjonalnie można monitorować czas oczekiwania na synchronizację
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET) {
        ESP_LOGI(TAG_NTP, "Waiting for time synchronization...");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    struct timeval tv;
    gettimeofday(&tv, NULL);
    ESP_LOGI(TAG_NTP, "Time synchronized: %ld sec | %ld usec", (long)tv.tv_sec, (long)tv.tv_usec);
}
