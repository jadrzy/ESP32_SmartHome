#include "components.h"
#include "components/nvs/nvs.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_wifi_netif.h"
#include "esp_wifi_types_generic.h"
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

static const char TAG_BODY[] = "BODY";

static wifi_sta_cred_t station_cerdentials;
static device_t master_device;
static device_t paired_devices[NUMBER_OF_DEVICES]; 

device_t get_master_device_cred(void) {return master_device;}
wifi_sta_cred_t get_station_cred(void) {return station_cerdentials;}

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

esp_err_t memory_setup(void)
{
    esp_err_t err = ESP_OK;
    ESP_ERROR_CHECK(init_nvs_partitions());

    ESP_ERROR_CHECK(err = get_master_serial_number_from_nvs(master_device.serial));
    if (err == ESP_OK){
        ESP_LOGI(TAG_BODY, "Serial number extracted from nvs...");
    }

    uint8_t mac[6];
    ESP_ERROR_CHECK(esp_efuse_mac_get_default(mac));
    mac_8_64(mac, &master_device.mac);
    if (err == ESP_OK){
        ESP_LOGI(TAG_BODY, "MAC address extracted...");
    }

    err = get_wifi_sm_cred_from_nvs(station_cerdentials.ssid, station_cerdentials.psswd);
    if (err == ESP_OK){
        ESP_LOGI(TAG_BODY, "WIFI station credentialse extracted from nvs...");
    }

    err = get_paired_devices_from_nvs(paired_devices);
    if (err == ESP_OK){
        ESP_LOGI(TAG_BODY, "Paired devices credentials extracted from nvs...");
    }

    return err;
}

void mac_8_64(const uint8_t input[6], uint64_t *output)
{
    for (int i = 0; i < 6; i++)
        *output = (*output << 8) | input[i];
}

void mac_64_8(const uint64_t input, uint8_t output[6])
{
    for (int i = 0; i < 6; i++)
        output[5 - i] = (uint8_t) (input >> (8 * i));
}
