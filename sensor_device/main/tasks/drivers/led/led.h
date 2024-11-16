#ifndef LED_HEADER
#define LED_HEADER

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"

#define LED_SIGNAL_PIN_MASK (1ULL << 23) 
#define LED_SIGNAL_PIN (GPIO_NUM_23) 
#define LED_BLINK_TIME (10 / portTICK_PERIOD_MS)

esp_err_t signal_led_init(void);
esp_err_t blink_signal_led(void);

#endif
