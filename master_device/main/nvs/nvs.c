#include "nvs.h"
#include "master_device.h"

// Initialize NVS partitions has to be called before read/write
void init_nvs_partitions(void)
{
    esp_err_t ret = ESP_OK;

    // Initialize NVS partition for "nvs"
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize NVS partition for "serial"
    ret = nvs_flash_init_partition("data");
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase_partition("data"));
        ret = nvs_flash_init_partition("data");
    }
    ESP_ERROR_CHECK(ret);
}

void get_master_serial_number_from_nvs(char *number)
{
    esp_err_t err;
    nvs_handle_t my_handle;

    // Open NVS handle for reading the master device serial number
    err = nvs_open_from_partition("data", "master_device", NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return;
    }

    // Get required size for the serial number 
    size_t required_size = 0; 
    err = nvs_get_str(my_handle, "serial_number", NULL, &required_size);
    ESP_ERROR_CHECK(err);

    // Read the serial number
    char string[SERIAL_NUMBER_SIZE + 1];
    err = nvs_get_str(my_handle, "serial_number", string, &required_size);
    ESP_ERROR_CHECK(err);

    // Remove the newline character if present
    string[strcspn(string, "\n")] = '\0';

    if (err == ESP_OK) {
        strcpy(number, string);
    }

    nvs_close(my_handle);
}


void get_wifi_sm_cred_from_nvs(char * SSID, char * PSSWD)
{
    esp_err_t err;
    nvs_handle_t my_handle;

    // Open NVS handle for reading the master device serial number
    err = nvs_open_from_partition("data", "WIFI_SM", NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return;
    }

    // Get required size for the SSID 
    size_t required_size = 0; 
    err = nvs_get_str(my_handle, "SSID", NULL, &required_size);
    ESP_ERROR_CHECK(err);

    // Read the serial number
    char string[WIFI_CREDENTIALS_SIZE + 1];
    err = nvs_get_str(my_handle, "SSID", string, &required_size);
    ESP_ERROR_CHECK(err);

    // Remove the newline character if present
    string[strcspn(string, "\n")] = '\0';

    if (err == ESP_OK) {
        strcpy(SSID, string);
    }

    // Get required size for the PSSWD 
    err = nvs_get_str(my_handle, "PSSWD", NULL, &required_size);
    ESP_ERROR_CHECK(err);

    // Read the serial number
    err = nvs_get_str(my_handle, "PSSWD", string, &required_size);
    ESP_ERROR_CHECK(err);

    // Remove the newline character if present
    string[strcspn(string, "\n")] = '\0';

    if (err == ESP_OK) {
        strcpy(PSSWD, string);
    }
    nvs_close(my_handle);
}


void get_slave_serial_number_from_nvs(char number[][SERIAL_NUMBER_SIZE + 1])
{
    esp_err_t err;
    nvs_handle_t my_handle;

    // Open NVS handle for reading the slave devices serial numbers
    err = nvs_open_from_partition("data", "slave_device", NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
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

void write_wifi_sm_cred_to_nvs(char * SSID,char * PSSWD)
{
    esp_err_t err;
    nvs_handle_t my_handle;

    // Open NVS handle for writing the slave devices serial numbers
    err = nvs_open_from_partition("data", "WIFI_SM", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return;
    }

    // Write the serial number to NVS
    err = nvs_set_str(my_handle, "SSID", SSID);
    ESP_ERROR_CHECK(err);

    // Write the serial number to NVS
    err = nvs_set_str(my_handle, "PSSWD", PSSWD);
    ESP_ERROR_CHECK(err);

    // Commit the changes
    err = nvs_commit(my_handle);
    ESP_ERROR_CHECK(err);
    nvs_close(my_handle);
}

void write_slave_serial_number_to_nvs(char number[][SERIAL_NUMBER_SIZE + 1])
{
    esp_err_t err;
    nvs_handle_t my_handle;

    // Open NVS handle for writing the slave devices serial numbers
    err = nvs_open_from_partition("data", "slave_device", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
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
