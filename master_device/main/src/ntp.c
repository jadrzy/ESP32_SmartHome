#include "include/ntp.h"
#include <bits/types/struct_timeval.h>
#include "include/wifi.h"

static const char* TAG_NTP = "NTP";


void time_sync_cb(struct timeval *tv)
{
    wifi_flags_t *flags;
    flags = get_wifi_flags();
    ESP_LOGI(TAG_NTP, "Time synchronized");
    flags->rtc_synchronized = 1;
}

time_t get_timestamp(void)
{

}
