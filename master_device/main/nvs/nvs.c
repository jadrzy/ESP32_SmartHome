#include "nvs.h"
#include <string.h>

static uint64_t assigned_devices[10] = {0};


void init_nvs(void)
{
esp_err_t ret = nvs_flash_init_partition("serial");
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init_partition("serial");
    }
    ESP_ERROR_CHECK(ret);
}

void operation(void)
{
    // Open
    esp_err_t err;
    nvs_handle_t my_handle;
    err = nvs_open_from_partition("serial", "master_device", NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        printf("Done\n");

    size_t required_size = 0;  // value will default to 0, if not set yet in NVS
    err = nvs_get_str(my_handle, "serial_number", NULL, &required_size);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) 
        {
            ;
        // Read
        }
        printf("Size = %d\n", (int) required_size);
        // printf("Reading serial number from NVS ... ");
        char serial_string[12]; // value will default to 0, if not set yet in NVS
        err = nvs_get_str(my_handle, "serial_number", serial_string, &required_size);
        switch (err) {
            case ESP_OK:
                printf("Done\n");
                printf("Serial_number = %s", serial_string);
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                printf("The value is not initialized yet!\n");
                break;
            default :
                printf("Error (%s) reading!\n", esp_err_to_name(err));
        }

        // // Write
        // printf("Updating restart counter in NVS ... ");
        // restart_counter++;
        // err = nvs_set_i32(my_handle, "restart_counter", restart_counter);
        // printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        // Commit written value.
        // After setting any values, nvs_commit() must be called to ensure changes are written
        // to flash storage. Implementations may write to storage at other times,
        // but this is not guaranteed.
        // printf("Committing updates in NVS ... ");
        // err = nvs_commit(my_handle);
        // printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
        //
        // Close
        nvs_close(my_handle);
    }
}
