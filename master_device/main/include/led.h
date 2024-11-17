#ifndef LED_HEADER
#define LED_HEADER

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"

#define LED_SIGNAL_PIN_MASK ( (1ULL << 4) | (1ULL << 16) )
#define LED_WIFI_PIN (GPIO_NUM_16)
#define LED_DATA_PIN (GPIO_NUM_4)

#define LED_DATA_BLINK_TIME (10 / portTICK_PERIOD_MS)

esp_err_t init_led(void);
esp_err_t toggle_wifi_led(void);
esp_err_t set_on_wifi_led(void);
esp_err_t blink_signal_led(void);


#endif
