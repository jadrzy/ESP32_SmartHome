#include "data.h"

static const char TAG_DATA[] = "DATA";

static slave_device_t device_cred;
static sensor_data_t sensor_data = {0};
static light_control_t light_control = {0};

double get_lux(void)
{
    return sensor_data.lux;
}

double get_temperature(void)
{
    return sensor_data.temperature;
}

double get_humidity(void)
{
    return sensor_data.humidity;
}

double get_pressure(void)
{
    return sensor_data.pressure;
}


bool get_light_mode(void)
{
    return light_control.auto_light;    
}


int get_light_value(void)
{
    return light_control.light_value;    
}


void get_slave_device(char serial[SERIAL_NUMBER_SIZE], uint8_t mac[6])
{
    strcpy(serial, device_cred.serial_number);
    memcpy(mac, device_cred.mac_address, 6);
}


void set_lux(double value)
{
    sensor_data.lux = value; 
}

void set_temperature(double value)
{
    sensor_data.temperature = value;
}

void set_humidity(double value)
{
    sensor_data.humidity = value;
}

void set_pressure(double value)
{
    sensor_data.pressure = value;
}

void set_light_mode(bool mode)
{
    light_control.auto_light = mode;    
}

void set_light_value(int value)
{
    light_control.light_value = value;
}

static void set_slave_device(const char serial[SERIAL_NUMBER_SIZE], const uint8_t mac[6])
{
    strcpy(device_cred.serial_number, serial);
    memcpy(device_cred.mac_address, mac, 6);
}


esp_err_t memory_setup(void)
{
    esp_err_t err = ESP_OK;
    ESP_ERROR_CHECK(init_nvs_partitions());

    char serial[SERIAL_NUMBER_SIZE];

    err = get_slave_serial_number_from_nvs(serial);
    if (err == ESP_OK){
        ESP_LOGI(TAG_DATA, "Serial number extracted from nvs...");
    }

    uint8_t mac[6];
    ESP_ERROR_CHECK(esp_efuse_mac_get_default(mac));
    if (err == ESP_OK){
        ESP_LOGI(TAG_DATA, "MAC address extracted...");
    }

    set_slave_device(serial, mac);

    return err;
}

void debug(void)
{
    char serial[SERIAL_NUMBER_SIZE];
    uint8_t mac[6];
    get_slave_device(serial, mac);

    ESP_LOGI(TAG_DATA, "Serial = %s", serial);
    ESP_LOGI(TAG_DATA, "Mac = %x:%x:%x:%x:%x:%x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);    
}

