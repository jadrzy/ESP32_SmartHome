#include "nvs.h"

// Initialize NVS partitions has to be called before read/write
void init_nvs_partitions(void)
{
    esp_err_t ret;

    // Initialize NVS partition for "nvs"
    ret = nvs_flash_init_partition("nvs");
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase_partition("nvs"));
        ret = nvs_flash_init_partition("nvs");
    }
    ESP_ERROR_CHECK(ret);

    // Initialize NVS partition for "serial"
    ret = nvs_flash_init_partition("serial");
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase_partition("serial"));
        ret = nvs_flash_init_partition("serial");
    }
    ESP_ERROR_CHECK(ret);
}

void get_slave_serial_number_from_nvs(char *number)
{
    esp_err_t err;
    nvs_handle_t my_handle;

    // Open NVS handle for reading the master device serial number
    err = nvs_open_from_partition("serial", "slave_device", NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return;
    }

    // Get required size for the serial number
    size_t required_size = 0; 
    err = nvs_get_str(my_handle, "serial_number", NULL, &required_size);
    ESP_ERROR_CHECK(err);

    // Read the serial number
    char serial_string[SERIAL_NUMBER_SIZE + 1];
    err = nvs_get_str(my_handle, "serial_number", serial_string, &required_size);
    ESP_ERROR_CHECK(err);

    // Remove the newline character if present
    serial_string[strcspn(serial_string, "\n")] = '\0';

    if (err == ESP_OK) {
        strcpy(number, serial_string);
    }

    nvs_close(my_handle);
}
