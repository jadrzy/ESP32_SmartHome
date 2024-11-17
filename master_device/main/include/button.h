#ifndef BUTTON_HEADER
#define BUTTON_HEADER
#include "freertos/projdefs.h"
#include "hal/gpio_types.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <stdint.h>
#include <esp_log.h>
#include <freertos/timers.h>
#include "include/wifi.h"


#define GPIO_SEL_23 (1ULL << 23)
#define PRESS_TIME (3 * 1000)   // 3s
//
void button_init(void);
void button_handler(void *);

void debouncer_callback_timer(TimerHandle_t xTimer);
void button_press_callback_timer(TimerHandle_t xTimer);

#endif
