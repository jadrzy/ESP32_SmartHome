#ifndef LIGHT_CONTROL_HEADER
#define LIGHT_CONTROL_HEADER

#include "esp_log.h"
#include "hal/gpio_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define GPIO_SEL_5 (1ULL << 5)
#define GPIO_SEL_18 (1ULL << 18)

bool * get_flag_zero_cossed(void);
void light_control_setup(void);
void light_toggle(unsigned int period);

#endif
