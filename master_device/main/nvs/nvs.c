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

void get_master_serial_number_from_nvs(char *number)
{
    esp_err_t err;
    nvs_handle_t my_handle;

    // Open NVS handle for reading the master device serial number
    err = nvs_open_from_partition("serial", "master_device", NVS_READONLY, &my_handle);
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

void get_slave_serial_number_from_nvs(char number[][SERIAL_NUMBER_SIZE + 1])
{
    esp_err_t err;
    nvs_handle_t my_handle;

    // Open NVS handle for reading the slave devices serial numbers
    err = nvs_open_from_partition("serial", "slave_device", NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return;
    }

    char serial_string[SERIAL_NUMBER_SIZE + 1]; 

    for (int i = 1; i <= NUMBER_OF_DEVICES; i++) {
        char key[12] = "serial_00";  // Adjust key size for proper usage
        snprintf(&key[7], sizeof(key) - 7, "%02d", i); // Generate key

        // Get required size for the serial number
        size_t required_size; 
        err = nvs_get_str(my_handle, key, NULL, &required_size);
        ESP_ERROR_CHECK(err);

        // Read the serial number
        err = nvs_get_str(my_handle, key, serial_string, &required_size);
        ESP_ERROR_CHECK(err);

        // Remove the newline character if present
        serial_string[strcspn(serial_string, "\n")] = '\0';

        if (err == ESP_OK) {
            strcpy(number[i - 1], serial_string);
        }
    }

    nvs_close(my_handle);
}

void write_slave_serial_number_to_nvs(char number[][SERIAL_NUMBER_SIZE + 1])
{
    esp_err_t err;
    nvs_handle_t my_handle;

    // Open NVS handle for writing the slave devices serial numbers
    err = nvs_open_from_partition("serial", "slave_device", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return;
    }

    for (int i = 1; i <= NUMBER_OF_DEVICES; i++) {
        char key[12] = "serial_00";  // Adjust key size for proper usage
        snprintf(&key[7], sizeof(key) - 7, "%02d", i); // Generate key

        // Write the serial number to NVS
        err = nvs_set_str(my_handle, key, number[i - 1]);
        ESP_ERROR_CHECK(err);

        // Commit the changes
        err = nvs_commit(my_handle);
        ESP_ERROR_CHECK(err);
    }

    nvs_close(my_handle);
}
