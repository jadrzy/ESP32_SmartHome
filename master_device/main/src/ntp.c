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
#include "nvs_flash.h"
#include "esp_sntp.h"
#include "esp_netif_types.h"


#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_netif_sntp.h"
#include "sdkconfig.h"


static const char* TAG_NTP = "NTP";

void time_sync_cb(struct timeval *tv)
{
    wifi_flags_t *flags = get_wifi_flags();
    ESP_LOGI(TAG_NTP, "Time synchronized");
    flags->rtc_synchronized = 1;
}

#include "esp_http_client.h"

static void http_test_task(void)
{
    esp_http_client_config_t config = {
        .url = "http://example.com",
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI("HTTP", "HTTP GET Status = %d, content_length = %d",
                 (int) esp_http_client_get_status_code(client),
                 (int) esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE("HTTP", "HTTP GET request failed: %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
    vTaskDelete(NULL);
}


void synch_time(void)
{
    ESP_LOGI(TAG_NTP, "Checking DNS resolution...");
    struct addrinfo *res;
    int err = getaddrinfo("example.com", NULL, NULL, &res);
    if (err != 0) {
        ESP_LOGE(TAG_NTP, "DNS lookup failed: %d", err);
        return;
    }
    struct sockaddr_in *addr = (struct sockaddr_in *)res->ai_addr;
    ESP_LOGI(TAG_NTP, "Resolved example.com to IP: " IPSTR, IP2STR(&addr->sin_addr));
    freeaddrinfo(res);

    ESP_LOGI(TAG_NTP, "Testing HTTP connection...");
    esp_http_client_config_t config = {
        .url = "http://example.com",
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG_NTP, "HTTP GET Status = %d, content_length = %d",
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG_NTP, "HTTP GET request failed: %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);

    ESP_LOGI(TAG_NTP, "Initializing SNTP...");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_cb);
    esp_sntp_init();

    ESP_LOGI(TAG_NTP, "SNTP initialized, waiting for synchronization...");
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    if (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET) {
        ESP_LOGE(TAG_NTP, "SNTP synchronization failed.");
        return;
    }

    struct timeval tv;
    gettimeofday(&tv, NULL);
    ESP_LOGI(TAG_NTP, "Time synchronized: %ld sec | %ld usec", (long)tv.tv_sec, (long)tv.tv_usec);
}
