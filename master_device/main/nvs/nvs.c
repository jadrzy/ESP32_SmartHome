#include "nvs.h"

void init_nvs_partitions(void)
{
    esp_err_t ret;
    ret = nvs_flash_init_partition("nvs");
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase_partition("nvs"));
        ret = nvs_flash_init_partition("nvs");
    }
    ESP_ERROR_CHECK(ret);

    ret = nvs_flash_init_partition("serial");
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase_partition("serial"));
        ret = nvs_flash_init_partition("serial");
    }
    ESP_ERROR_CHECK(ret);
}


void get_master_serial_number_from_nvs(char * number)
{
    esp_err_t err;
    nvs_handle_t my_handle;

    err = nvs_open_from_partition("serial", "master_device", NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {

        // GET REQUIERED SIZE
        size_t required_size = 0; 
        err = nvs_get_str(my_handle, "serial_number", NULL, &required_size);
        ESP_ERROR_CHECK(err);

        // READ
        char serial_string[SERIAL_NUMBER_SIZE + 1];
        err = nvs_get_str(my_handle, "serial_number", serial_string, &required_size);
        ESP_ERROR_CHECK(err);

        // Remove the newline character if present
        serial_string[strcspn(serial_string, "\n")] = '\0';

        if (err == ESP_OK) 
        {
            strcpy(number, serial_string);
        }
    }
    nvs_close(my_handle);
}


void get_slave_serial_number_from_nvs(char number[][SERIAL_NUMBER_SIZE + 1])
{
    esp_err_t err;
    nvs_handle_t my_handle;

    err = nvs_open_from_partition("serial", "slave_device", NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {

        size_t required_size; 
        char serial_string[SERIAL_NUMBER_SIZE + 1]; 

        for (int i = 1; i <= NUMBER_OF_DEVICES; i++)
        {
            char key[] = "serial_00";
            char buffer[3]; 

            // PARSING KEY
            if (i < 10)
            {
                sprintf(buffer, "%02d", i);
            }
            else
            {
                sprintf(buffer, "%d", i);
            }
            strncpy(&key[strcspn(key, "00")], buffer, sizeof(buffer));

            // GET REQUIERED SIZE
            err = nvs_get_str(my_handle, key, NULL, &required_size);

            // READ
            err = nvs_get_str(my_handle, key, serial_string, &required_size);
            ESP_ERROR_CHECK(err);

            // Remove the newline character if present
            serial_string[strcspn(serial_string, "\n")] = '\0';

            if (err == ESP_OK) 
            {
                strcpy(number[i - 1], serial_string);
            }
        }
    }
    nvs_close(my_handle);
}


void write_slave_serial_number_to_nvs(char number[][SERIAL_NUMBER_SIZE + 1])
{
    esp_err_t err;
    nvs_handle_t my_handle;

    err = nvs_open_from_partition("serial", "slave_device", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        
        for (int i = 1; i <= NUMBER_OF_DEVICES; i++) 
        {
            char key[] = "serial_00";
            char buffer[3]; 

            // PARSING KEY
            if (i < 10)
            {
                sprintf(buffer, "%02d", i);
            }
            else
            {
                sprintf(buffer, "%d", i);
            }
            strncpy(&key[strcspn(key, "00")], buffer, sizeof(buffer));


            err = nvs_set_str(my_handle, key, number[i - 1]);
            ESP_ERROR_CHECK(err);
            err = nvs_commit(my_handle);
            ESP_ERROR_CHECK(err);
        }
    }

    nvs_close(my_handle);
}
