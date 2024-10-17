#include "components.h"
#include "components/nvs/nvs.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_wifi_netif.h"
#include "esp_wifi_types_generic.h"
#include <string.h>
#include <inttypes.h>

static const char TAG_BODY[] = "BODY";

static wifi_sta_cred_t station_cerdentials;
static device_t paired_devices[NUMBER_OF_DEVICES]; 



static void debug_nvs(void)
{
    ESP_LOGI(TAG_BODY, "Station credentials:");
    ESP_LOGI(TAG_BODY, "SSID = %s", station_cerdentials.ssid);
    ESP_LOGI(TAG_BODY, "PSSWD = %s", station_cerdentials.psswd);
    ESP_LOGI(TAG_BODY, "Paired devices credentials:");
    for (int i = 0; i < NUMBER_OF_DEVICES; i++)
    {
        ESP_LOGI(TAG_BODY, "Device %d:\tserial = %s\tmac = %" PRIu64, (i + 1), paired_devices[i].serial, paired_devices[i].mac);
    }
}



esp_err_t run(void)
{
    esp_err_t err = ESP_OK;
    err = get_wifi_sm_cred_from_nvs(station_cerdentials.ssid, station_cerdentials.psswd);
    if (err == ESP_OK){
        ESP_LOGI(TAG_BODY, "WIFI station credentials were extracted from nvs...");
    }

    err = get_paired_devices_from_nvs(paired_devices);
    if (err == ESP_OK){
        ESP_LOGI(TAG_BODY, "Paired devices credentials were extracted from nvs...");
    }

    debug_nvs();

    strcpy(station_cerdentials.ssid, "new_ssid");
    strcpy(station_cerdentials.psswd, "new_psswd");

    for (int i = 0; i < NUMBER_OF_DEVICES; i++)
    {
        int num = paired_devices[i].mac;
        num = num + 1;
        paired_devices[i].mac = num;
    }

    strcpy(paired_devices[6].serial, "new_serial");
    strcpy(paired_devices[9].serial, "new_serial");

    err = write_wifi_sm_cred_to_nvs(station_cerdentials.ssid, station_cerdentials.psswd);
    if (err == ESP_OK){
        ESP_LOGI(TAG_BODY, "WIFI station credentials saved in nvs...");
    }

    err = write_paired_devices_to_nvs(paired_devices);
    if (err == ESP_OK){
        ESP_LOGI(TAG_BODY, "Paired devices credentials saved in nvs...");
    }

    return err;
}
