#include "led.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include <stdio.h>
#include "esp_log.h"
#include "freertos/idf_additions.h"

static const char* TAG_LED = "LED_DRIVER";

esp_err_t signal_led_init(void)
{
    esp_err_t err = ESP_OK;

    gpio_config_t conf = {
        .mode = GPIO_MODE_OUTPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pin_bit_mask = LED_SIGNAL_PIN_MASK
    };
    err = gpio_config(&conf);
    if (err == ESP_OK)
        ESP_LOGI(TAG_LED, "Status led initialized");

    gpio_set_level(LED_SIGNAL_PIN, 0);
    return err;
}

esp_err_t blink_signal_led(void)
{
    esp_err_t err = ESP_OK;

    err = gpio_set_level(LED_SIGNAL_PIN, 1);
    ESP_ERROR_CHECK_WITHOUT_ABORT(err);

    vTaskDelay(LED_BLINK_TIME);

    gpio_set_level(LED_SIGNAL_PIN, 0);
    ESP_ERROR_CHECK_WITHOUT_ABORT(err);
    return err;
}
