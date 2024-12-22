#include "tasks.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "tasks/drivers/led/led.h"
#include "tasks/wifi/wifi.h"
#include "tasks/data/data.h"
#include <stdint.h>
#include "drivers/light_control/light_control.h"




static const char TAG_TASK[] = "TASK";

// Static handles for I2C and task management
static struct i2c i2c_handles;
static struct task_handles task_handles;
static TimerHandle_t channel_sniffer_timer = NULL;

static unsigned int light_period = 0;

// Mutex handles for sensor data protection
static SemaphoreHandle_t 
    xMutex_Lux,
    xMutex_Humidity,
    xMutex_Temperature,
    xMutex_Pressure,
    xMutex_Light_settings,
    xMutex_Light_period;


// Function to initialize all sensors
void initialize_sensors(void)
{
    // Initialize I2C and configure sensors
    i2c_init(&i2c_handles);
    light_sensor_config(&i2c_handles.light_sensor_handle);
    ESP_LOGI(TAG_TASK, "Light sensor acquisition period = %d ms", (int) LIGHT_SENSOR_MEASUREMENT_TIME);

    temp_hum_sensor_config(&i2c_handles.temp_hum_sensor_handle);
    ESP_LOGI(TAG_TASK, "Temperature and humidity sensor acquisition period = %d ms", (int) TEMP_HUM_SENSOR_MEASUREMENT_TIME + 85);

    pressure_sensor_config(&i2c_handles.pressure_sensor_handle);
    ESP_LOGI(TAG_TASK, "Pressure sensor acquisition period = %d ms", (int) PRESSURE_SENSOR_MEASUREMENT_TIME + 45);
}

// Task function to read temperature and humidity values
void task_fun_get_temp_hum_values(void *p)
{
    double humidity_value, temperature_value;

    while (1)
    {
        // Read temperature and humidity sensor data
        read_temp_hum_sensor(&i2c_handles.temp_hum_sensor_handle, &humidity_value, &temperature_value);

        // Protect and update humidity value with mutex
        if (xSemaphoreTake(xMutex_Humidity, 10) == pdTRUE)
        {
            set_humidity(humidity_value);
            xSemaphoreGive(xMutex_Humidity);
        }

        // Protect and update temperature value with mutex
        if (xSemaphoreTake(xMutex_Temperature, 10) == pdTRUE)
        {
            set_temperature(temperature_value);
            xSemaphoreGive(xMutex_Temperature);
        }

        // Delay for the next measurement
        vTaskDelay(TEMP_HUM_SENSOR_MEASUREMENT_TIME);
    }
}

// Task function to read light sensor (lux) values
void task_fun_get_lux_value(void *p)
{
    double lux_value;

    while (1)
    {
        // Read light sensor data
        read_light_sensor(&i2c_handles.light_sensor_handle, &lux_value);

        // Protect and update lux value with mutex
        if (xSemaphoreTake(xMutex_Lux, 10) == pdTRUE)
        {
            set_lux(lux_value);
            xSemaphoreGive(xMutex_Lux);
        }

        // Delay for the next measurement
        vTaskDelay(LIGHT_SENSOR_MEASUREMENT_TIME);
    }
}

// Task function to read pressure values
void task_fun_get_pressure_value(void *p)
{
    double pressure_value;

    while (1)
    {
        // Read pressure sensor data
        read_pressure_sensor(&i2c_handles.pressure_sensor_handle, &pressure_value);

        // Protect and update pressure value with mutex
        if (xSemaphoreTake(xMutex_Pressure, 10) == pdTRUE)
        {
            set_pressure(pressure_value);
            xSemaphoreGive(xMutex_Pressure);
        }

        // Delay for the next measurement
        vTaskDelay(PRESSURE_SENSOR_MEASUREMENT_TIME);
    }
}

void task_adjust_light_control_period(void *p)
{
    uint8_t automatic = 0;
    int value = 0; // in range (0-100)
    
    int new_period = 0; 

    int current_lux = 0;
    int difference = 0;

    while(1)
    {
        // Protect and update pressure value with mutex
        if (xSemaphoreTake(xMutex_Light_settings, 10) == pdTRUE)
        {
            automatic = get_light_mode();
            value = get_light_value();
            xSemaphoreGive(xMutex_Light_settings);
        }

        if (value > 100 || value < 0)
        {
            ESP_LOGI(TAG_TASK, "Wrong light value = %d", value);
            continue;
        }

        if (automatic == true)  // AUTOMATIC MODE
        {
            if (xSemaphoreTake(xMutex_Lux, 10) == pdTRUE)
            {
                current_lux = get_lux();
                xSemaphoreGive(xMutex_Lux);
            }


            difference = value - current_lux;


            if (difference > 0)
            {
                if (difference >= FUZZY_BIG)
                   new_period -= FUZZY_BIG * 10; 
                else if (difference < FUZZY_BIG || difference >= FUZZY_MEDIUM)
                   new_period -= FUZZY_MEDIUM * 10; 
                else if (difference < FUZZY_MEDIUM || difference >= FUZZY_SMALLx1)
                   new_period -= FUZZY_SMALLx1 * 10; 
                else if (difference < FUZZY_SMALLx1 || difference >= FUZZY_SMALLx2)
                   new_period -= FUZZY_SMALLx2 * 10; 
                else if (difference > FUZZY_SMALLx2 || difference >= FUZZY_SMALLx3)
                   new_period -= FUZZY_SMALLx3 * 10; 
            }
            else 
            {
                difference = abs(difference);

                if (difference >= FUZZY_BIG)
                   new_period += FUZZY_BIG * 10; 
                else if (difference < FUZZY_BIG || difference >= FUZZY_MEDIUM)
                   new_period += FUZZY_MEDIUM * 10; 
                else if (difference < FUZZY_MEDIUM || difference >= FUZZY_SMALLx1)
                   new_period += FUZZY_SMALLx1 * 10; 
                else if (difference < FUZZY_SMALLx1 || difference >= FUZZY_SMALLx2)
                   new_period += FUZZY_SMALLx2 * 10; 
                else if (difference > FUZZY_SMALLx2 || difference >= FUZZY_SMALLx3)
                   new_period += FUZZY_SMALLx3 * 10; 
            }

            if (new_period < 0)
                new_period = 0;
            if (new_period > 10000)
                new_period = 10000;

            if (xSemaphoreTake(xMutex_Light_period, 10) == pdTRUE)
            {
                light_period = new_period;
                xSemaphoreGive(xMutex_Light_period);
            }
            vTaskDelay(LIGHT_SENSOR_MEASUREMENT_TIME * 4);
        }
        else                    // MANUAL MODE
        {
            new_period = 10000 - (value * 100);
            if (xSemaphoreTake(xMutex_Light_period, 10) == pdTRUE)
            {
                light_period = new_period;
                xSemaphoreGive(xMutex_Light_period);
            }


            vTaskDelay(MANUAL_LIGHT_REFRESH);
        }
    }
}

unsigned int *get_light_period(void)
{
    return &light_period;
}


void task_channel_sniffer(void *p)
{
    wifi_flags_t *flags;
    flags = get_wifi_flags();
    while(1)
    {
        uint8_t new_channel = 0;
        uint8_t primary;
        wifi_second_chan_t secondary;

        if (flags->wifi_initialized == true || flags->esp_now_initiated == true)
        {
            ESP_ERROR_CHECK(esp_wifi_get_channel(&primary, &secondary));
            new_channel = (primary < WIFI_MAX_CHANNEL) ? (primary + 1) : 1; 
            ESP_LOGI(TAG_TASK, "NEW_CHANNEL = %d", new_channel);
            ESP_ERROR_CHECK(esp_wifi_set_channel(new_channel, WIFI_SECOND_CHAN_NONE));
        }

        vTaskDelay(SNIFFER_TASK_PERIOD);
    }
}


void sniffer_suspend(void)
{
    vTaskSuspend(task_handles.channel_sniffer_task);
    ESP_LOGI(TAG_TASK, "Channel sniffer suspended");
    xTimerStart(channel_sniffer_timer, 5);
}

void sniffer_activate(void)
{
    vTaskResume(task_handles.channel_sniffer_task);
    ESP_LOGI(TAG_TASK, "Channel sniffer activated");
    xTimerStop(channel_sniffer_timer, 5);
}

void reset_sniffer_timer(void)
{
    xTimerReset(channel_sniffer_timer, 5);
    ESP_LOGI(TAG_TASK, "Timer reset"); 
}

void callback_sniffer_timer(TimerHandle_t xTimer)
{
    ESP_LOGI(TAG_TASK, "Max connection idle time reached...");
    delete_my_peer();
    ESP_LOGI(TAG_TASK, "Peer deleted");
    sniffer_activate();
}


static esp_err_t handle_recv_queue_data(recv_data_t * data)
{
    esp_err_t err = ESP_OK;
    //mutex

    ESP_LOGI(TAG_TASK, "Recieved data: lm = %d | light_value = %d", data->auto_light, data->light_value);
    if (xSemaphoreTake(xMutex_Light_settings, 10) == pdTRUE)
    {
        set_light_mode(data->auto_light);
        set_light_value(data->light_value);
        xSemaphoreGive(xMutex_Light_settings);
    }
    return err;
}

static esp_err_t prepare_send_data(send_data_t *data)
{
    esp_err_t err = ESP_OK;

    get_slave_device(data->serial, data->mac_address);

    // Read and log lux value
    if (xSemaphoreTake(xMutex_Lux, 10) == pdTRUE)
    {
        data->lux = get_lux();
        xSemaphoreGive(xMutex_Lux);
    }

    // Read and log humidity value
    if (xSemaphoreTake(xMutex_Humidity, 10) == pdTRUE)
    {
        data->humidity = get_humidity();
        xSemaphoreGive(xMutex_Humidity);
    }

    // Read and log temperature value
    if (xSemaphoreTake(xMutex_Temperature, 10) == pdTRUE)
    {
        data->temperature = get_temperature();
        xSemaphoreGive(xMutex_Temperature);
    }

    // Read and log pressure value
    if (xSemaphoreTake(xMutex_Pressure, 10) == pdTRUE)
    {
        data->pressure = get_pressure();
        xSemaphoreGive(xMutex_Pressure);
    }

    ESP_LOGI(TAG_TASK, "Sending data: lux = %g | pressure = %g | humidity = %g | temperature = %g", 
             data->lux, data->pressure, data->humidity, data->temperature);

    return err;
}



static QueueHandle_t recieve_data_queue;

QueueHandle_t get_queue_handle(void)
{
    return recieve_data_queue;
}

void recv_queue_task(void *p)
{
    esp_err_t err = ESP_OK;
    static recv_data_t recv_data;
    static send_data_t send_data;  

    ESP_LOGI(TAG_TASK, "Recieve queue task started");

    while(1)
    {
        if(xQueueReceive(recieve_data_queue, &recv_data, portMAX_DELAY) == pdTRUE)
        {
            handle_recv_queue_data(&recv_data);

            err = prepare_send_data(&send_data);
            if (err != ESP_OK)
            {
                ESP_LOGI(TAG_TASK, "Error transforming data to be sent");
                continue;
            }

            send_espnow_data(send_data);
        }
    }
}


// Debug task to log sensor data
void task_debug(void *p)
{
    sensor_data_t sensor_data = {0};

    while (1)
    {
        // Read and log lux value
        if (xSemaphoreTake(xMutex_Lux, 10) == pdTRUE)
        {
            sensor_data.lux = get_lux();
            xSemaphoreGive(xMutex_Lux);
        }

        // Read and log humidity value
        if (xSemaphoreTake(xMutex_Humidity, 10) == pdTRUE)
        {
            sensor_data.humidity = get_humidity();
            xSemaphoreGive(xMutex_Humidity);
        }

        // Read and log temperature value
        if (xSemaphoreTake(xMutex_Temperature, 10) == pdTRUE)
        {
            sensor_data.temperature = get_temperature();
            xSemaphoreGive(xMutex_Temperature);
        }

        // Read and log pressure value
        if (xSemaphoreTake(xMutex_Pressure, 10) == pdTRUE)
        {
            sensor_data.pressure = get_pressure();
            xSemaphoreGive(xMutex_Pressure);
        }

        // Log all sensor data
        ESP_LOGI("SENSOR_DATA", "Lux: %g, Humidity: %g, Temperature: %g, Pressure: %g", 
                 sensor_data.lux, sensor_data.humidity, sensor_data.temperature, sensor_data.pressure);

        // Delay for logging
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

// Function to initialize and create tasks
void initialize_tasks(void)
{
    // Create mutexes for sensor data protection
    xMutex_Lux = xSemaphoreCreateMutex();    
    xMutex_Humidity = xSemaphoreCreateMutex();
    xMutex_Temperature = xSemaphoreCreateMutex();
    xMutex_Pressure = xSemaphoreCreateMutex();
    xMutex_Light_settings = xSemaphoreCreateMutex();
    xMutex_Light_period = xSemaphoreCreateMutex();

    recieve_data_queue = xQueueCreate(2, sizeof(recv_data_t));

    light_control_setup();

    // Create tasks for reading sensors and logging data
    xTaskCreatePinnedToCore(task_fun_get_lux_value, "Get_Lux_Task", 6144, NULL, 5, &task_handles.lux_task, 1);
    xTaskCreatePinnedToCore(task_fun_get_temp_hum_values, "Get_Temp_Hum_Task", 6144, NULL, 5, &task_handles.temp_hum_task, 1);
    xTaskCreatePinnedToCore(task_fun_get_pressure_value, "Get_Pressure_Task", 6144, NULL, 5, &task_handles.press_task, 1);
    xTaskCreatePinnedToCore(recv_queue_task, "Handle_rec_data", 6144, NULL, 6, &task_handles.recieve_data_task, 1);
    xTaskCreatePinnedToCore(task_channel_sniffer, "Toggle wifi channel", 4096, NULL, 4, &task_handles.channel_sniffer_task, 1);
    // xTaskCreatePinnedToCore(task_light_control, "Light control task", 4096, NULL, 10, &task_handles.light_control_task, 1);
    xTaskCreatePinnedToCore(task_adjust_light_control_period, "Light period adjust task", 4096, NULL, 5, &task_handles.light_period_adjust_task, 1);

    channel_sniffer_timer = xTimerCreate(
        "Disconnect timer",
        pdMS_TO_TICKS(RESET_PEER_TIME),
        pdFALSE,
        (void*)0,
        callback_sniffer_timer
    );
    //xTaskCreatePinnedToCore(task_debug, "Debug_Task", 4096, NULL, 5, NULL, 1);

}
