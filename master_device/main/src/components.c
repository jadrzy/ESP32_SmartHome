#include "include/components.h"
#include "esp_log.h"
#include <stdint.h>

static const char TAG_BODY[] = "BODY";


esp_err_t memory_setup(void)
{
    esp_err_t err = ESP_OK;
    ESP_ERROR_CHECK(init_nvs_partitions());

    char serial[SERIAL_NUMBER_SIZE];

    ESP_ERROR_CHECK(err = get_master_serial_number_from_nvs(serial));
    if (err == ESP_OK){
        ESP_LOGI(TAG_BODY, "Serial number extracted from nvs...");
    }

    uint8_t mac[6];
    ESP_ERROR_CHECK(esp_efuse_mac_get_default(mac));
    if (err == ESP_OK){
        ESP_LOGI(TAG_BODY, "MAC address extracted...");
    }

    set_master_device(serial, mac);

    char url[48];
    char api_key[148];
    ESP_ERROR_CHECK(err = get_http_cred_form_nvs(url, api_key));
    if (err == ESP_OK){
        ESP_LOGI(TAG_BODY, "HTTP credentials extracted...");
    }   
    
    set_http_cred(url, api);
    
    memory_update();

    return err;
}

esp_err_t memory_update(void)
{
    esp_err_t err = ESP_OK;

    slave_device_t devices[NUMBER_OF_DEVICES] = {0};

    char serial_numbers[NUMBER_OF_DEVICES][SERIAL_NUMBER_SIZE];
    uint64_t mac_addresses[NUMBER_OF_DEVICES];


    err = get_paired_devices_from_nvs(mac_addresses, serial_numbers);
    if (err == ESP_OK){
        ESP_LOGI(TAG_BODY, "Paired devices credentials extracted from nvs...");
    }

    for (int i = 0; i < NUMBER_OF_DEVICES; i++)
    {

    uint64_t macc;

        devices[i].new_data = false;

        if (mac_addresses[i])
        {
            devices[i].active = true;
            strcpy(devices[i].serial_number, serial_numbers[i]);

            uint8_t mac[6];
            mac_64_8(mac_addresses[i], mac);

            memcpy(devices[i].mac_address, mac, 6);
        }
        else
        {
            devices[i].active = false;
        }

        mac_8_64(devices[i].mac_address, &macc);

    }
    set_slave_devices(devices);

    return err;
}

void mac_8_64(const uint8_t input[6], uint64_t *output)
{
    *output = 0;
    for (int i = 0; i < 6; i++)
    {
        *output = (*output << 8) | input[i];
    }

}


void mac_64_8(const uint64_t input, uint8_t output[6])
{
    for (int i = 0; i < 6; i++)
        output[5 - i] = (uint8_t) (input >> (8 * i));
}
