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
    uint8_t mac[6];
    uint8_t cheese[6] = {0};
    ESP_ERROR_CHECK(esp_efuse_mac_get_default(mac));
    ESP_LOGI(TAG_BODY, "mac = %x;%x;%x;%x;%x;%x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    mac_8_64(mac, &master_device.mac);
    ESP_LOGI(TAG_BODY, "mac = %" PRIu64, master_device.mac);
    mac_64_8(master_device.mac, cheese);
    ESP_LOGI(TAG_BODY, "mac = %x;%x;%x;%x;%x;%x", cheese[0], cheese[1], cheese[2], cheese[3], cheese[4], cheese[5]);


    // err = get_wifi_sm_cred_from_nvs(station_cerdentials.ssid, station_cerdentials.psswd);
    // if (err == ESP_OK){
    //     ESP_LOGI(TAG_BODY, "WIFI station credentials were extracted from nvs...");
    // }
    return err;
}

void mac_8_64(const uint8_t input[6], uint64_t *output)
{
    uint64_t placeholder = 0;
    for (int i = 0; i < 6; i++){
            placeholder = placeholder << 8 || input[i];
        ESP_LOGI(TAG_BODY, "output %" PRIu64, placeholder);
    }
    *output = placeholder;
}

void mac_64_8(const uint64_t input, uint8_t output[6])
{
    for (int i = 5; i <= 0; i--)
        output[i] = (uint8_t) (input >> (8 * i));
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
