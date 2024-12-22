#include "include/nvs.h"

static const char TAG_NVS[] = "NVS";

// Initialize NVS partitions has to be called before read/write
esp_err_t init_nvs_partitions(void)
{
    esp_err_t ret = ESP_OK;

    // Initialize NVS partition for "nvs"
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize encrypted NVS partition
    // Find nvs_keys partition
    const esp_partition_t * partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS_KEYS, "nvs_keys");
    if (partition == NULL){
        ESP_LOGE(TAG_NVS, "Could not locate partition %s", "nvs_keys");
        return ret;
    }

    // Read nvs_keys partition
    nvs_sec_cfg_t cfg;
    ret = nvs_flash_read_security_cfg(partition, &cfg);
    ESP_ERROR_CHECK(ret); 

    // Initialize nvs partition
    ret = nvs_flash_secure_init_partition("data", &cfg);
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase_partition("data"));
        ret = nvs_flash_secure_init_partition("data", &cfg);
    }
    ESP_ERROR_CHECK(ret);
    return ret;
}


esp_err_t get_http_cred_from_nvs(char *url, char *api_key)
{
    esp_err_t err = ESP_OK;
    nvs_handle_t my_handle;

    // Open NVS handle
    err = nvs_open_from_partition("data", "HTTP", NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_NVS, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return err;
    }

    // Get required size for url
    size_t required_size = 0; 
    err = nvs_get_str(my_handle, "url", NULL, &required_size);
    ESP_ERROR_CHECK(err);

    // Read the url
    char string[64];
    err = nvs_get_str(my_handle, "url", string, &required_size);
    ESP_ERROR_CHECK(err);

    // Remove the newline character if present
    string[strcspn(string, "\n")] = '\0';

    if (err == ESP_OK) {
        strcpy(url, string);
    }

    // Get required size for api_key
    required_size = 0; 
    err = nvs_get_str(my_handle, "API_KEY", NULL, &required_size);
    ESP_ERROR_CHECK(err);

    // Read the url
    char api[148];
    err = nvs_get_str(my_handle, "API_KEY", api, &required_size);
    ESP_ERROR_CHECK(err);

    if (err == ESP_OK) {
        strcpy(api_key, api);
    }
 
    nvs_close(my_handle);
    return err;
}


esp_err_t get_master_serial_number_from_nvs(char *number)
{
    esp_err_t err = ESP_OK;
    nvs_handle_t my_handle;

    // Open NVS handle for reading the master device serial number
    err = nvs_open_from_partition("data", "master_device", NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_NVS, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return err;
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
    return err;
}


esp_err_t get_wifi_sm_cred_from_nvs(char * SSID, char * PSSWD)
{
    esp_err_t err = ESP_OK;
    nvs_handle_t my_handle;

    // Open NVS handle for reading the master device serial number
    err = nvs_open_from_partition("data", "WIFI_SM", NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_NVS, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return err;
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

    // Read psswd 
    err = nvs_get_str(my_handle, "PSSWD", string, &required_size);
    ESP_ERROR_CHECK(err);

    // Remove the newline character if present
    string[strcspn(string, "\n")] = '\0';

    if (err == ESP_OK) {
        strcpy(PSSWD, string);
    }
    nvs_close(my_handle);
    return err;
}


esp_err_t write_wifi_sm_cred_to_nvs(char * SSID,char * PSSWD)
{
    esp_err_t err = ESP_OK;
    nvs_handle_t my_handle;

    // Open NVS handle for writing the slave devices serial numbers
    err = nvs_open_from_partition("data", "WIFI_SM", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_NVS, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return err;
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
    return err;
}


esp_err_t get_paired_devices_from_nvs(uint64_t mac_device_list[NUMBER_OF_DEVICES], char serial_device_list[NUMBER_OF_DEVICES][SERIAL_NUMBER_SIZE])
{
    esp_err_t err = ESP_OK;
    nvs_handle_t my_handle_serial;
    nvs_handle_t my_handle_mac;

    // Open NVS handle for reading the slave devices serial numbers
    err = nvs_open_from_partition("data", "slave_device_s", NVS_READONLY, &my_handle_serial);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_NVS, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return err;
    }

    // Open NVS handle for reading the slave devices mac addresses 
    err = nvs_open_from_partition("data", "slave_device_m", NVS_READONLY, &my_handle_mac);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_NVS, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return err;
    }

    char serial_string[SERIAL_NUMBER_SIZE]; 
    uint64_t mac_address;

    for (int i = 1; i <= NUMBER_OF_DEVICES; i++) {
        char key_serial[12] = "serial_00";  // Adjust key size for proper usage
        snprintf(&key_serial[7], sizeof(key_serial) - 7, "%02d", i); // Generate key

        // Get required size for the serial number
        size_t required_size; 
        err = nvs_get_str(my_handle_serial, key_serial, NULL, &required_size);
        ESP_ERROR_CHECK(err);

        // Read the serial number
        err = nvs_get_str(my_handle_serial, key_serial, serial_string, &required_size);
        ESP_ERROR_CHECK(err);

        // Remove the newline character if present
        serial_string[strcspn(serial_string, "\n")] = '\0';
        strcpy(serial_device_list[i - 1], serial_string);

        // Get mac address
        char key_mac[9] = "mac_00";  // Adjust key size for proper usage
        snprintf(&key_mac[4], sizeof(key_mac) - 4, "%02d", i); // Generate key

        // Read mac address
        err = nvs_get_u64(my_handle_mac, key_mac, &mac_address);
        ESP_ERROR_CHECK(err);

        mac_device_list[i - 1] = mac_address;
    }

    nvs_close(my_handle_serial);
    nvs_close(my_handle_mac);
    return err;
}

esp_err_t write_paired_devices_to_nvs(uint64_t mac_device_list[NUMBER_OF_DEVICES], char serial_device_list[NUMBER_OF_DEVICES][SERIAL_NUMBER_SIZE])
{
    esp_err_t err = ESP_OK;
    nvs_handle_t my_handle_serial;
    nvs_handle_t my_handle_mac;

    // Open NVS handle for writing the slave devices serial numbers
    err = nvs_open_from_partition("data", "slave_device_s", NVS_READWRITE, &my_handle_serial);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_NVS, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return err;
    }

    // Open NVS handle for writing the slave devices mac addresses 
    err = nvs_open_from_partition("data", "slave_device_m", NVS_READWRITE, &my_handle_mac);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_NVS, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return err;
    }

    for (int i = 1; i <= NUMBER_OF_DEVICES; i++) {
        char key_s[12] = "serial_00";  // Adjust key size for proper usage
        snprintf(&key_s[7], sizeof(key_s) - 7, "%02d", i); // Generate key

        // Write the serial number to NVS
        err = nvs_set_str(my_handle_serial, key_s, serial_device_list[i - 1]);
        ESP_ERROR_CHECK(err);

        // Commit the changes
        err = nvs_commit(my_handle_serial);
        ESP_ERROR_CHECK(err);

        // Mac address
        char key_m[9] = "mac_00";  // Adjust key size for proper usage
        snprintf(&key_m[4], sizeof(key_m) - 4, "%02d", i); // Generate key

        // Write the mac address to NVS
        err = nvs_set_u64(my_handle_mac, key_m, mac_device_list[i - 1]);
        ESP_ERROR_CHECK(err);

        // Commit the changes
        err = nvs_commit(my_handle_mac);
        ESP_ERROR_CHECK(err);
    }

    nvs_close(my_handle_serial);
    nvs_close(my_handle_mac);
    return err;
}

