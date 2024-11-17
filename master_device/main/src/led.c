#include "include/led.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include <stdint.h>
#include <stdio.h>
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "hal/gpio_types.h"

static const char* TAG_LED = "LED DRIVER";
static uint8_t led_pin_state = 0;

esp_err_t init_led(void)
{
    esp_err_t err = ESP_OK;

    gpio_config_t conf = {
        .mode = GPIO_MODE_OUTPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pin_bit_mask = LED_SIGNAL_PIN_MASK
    };
    err = gpio_config(&conf);

    if (err == ESP_OK)
        ESP_LOGI(TAG_LED, "Led driver initialized");

    gpio_set_level(LED_WIFI_PIN, 0);
    gpio_set_level(LED_DATA_PIN, 0);

    return err;
}


esp_err_t toggle_wifi_led(void)
{
    esp_err_t err = ESP_OK;

    led_pin_state = !led_pin_state;
    err = gpio_set_level(LED_WIFI_PIN, led_pin_state);
    ESP_ERROR_CHECK_WITHOUT_ABORT(err);

    return err;
}

esp_err_t set_on_wifi_led(void)
{
    esp_err_t err = ESP_OK;

    err = gpio_set_level(LED_WIFI_PIN, 1);
    ESP_ERROR_CHECK_WITHOUT_ABORT(err);

    return err;
}

esp_err_t blink_signal_led(void)
{
    esp_err_t err = ESP_OK;

    err = gpio_set_level(LED_DATA_PIN, 1);
    ESP_ERROR_CHECK_WITHOUT_ABORT(err);

    vTaskDelay(LED_DATA_BLINK_TIME);

    err = gpio_set_level(LED_DATA_PIN, 0);
    ESP_ERROR_CHECK_WITHOUT_ABORT(err);

    return err;
}
