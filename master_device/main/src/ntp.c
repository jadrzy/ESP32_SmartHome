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
    wifi_flags_t *flags;
    flags = get_wifi_flags();
    ESP_LOGI(TAG_NTP, "Time synchronized");
    flags->rtc_synchronized = 1;
}

void synch_time(void)
{
    struct timeval tv;

    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_cb);
    sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
    ESP_LOGI(TAG_NTP, "timeval before %d | %d", (int) tv.tv_sec, (int) tv.tv_usec);
    sntp_sync_time(&tv);
    ESP_LOGI(TAG_NTP, "timeval after %d | %d", (int) tv.tv_sec, (int) tv.tv_usec);

    settimeofday(&tv, NULL);

    char strftime_buf[64];
    time_t now = 0;
    struct tm timeinfo = {0};
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG_NTP, "The current date/time in Cracow is: %s", strftime_buf);
}
