#ifndef LIGHT_CONTROL_HEADER
#define LIGHT_CONTROL_HEADER

#include "driver/gpio.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "hal/gpio_types.h"
#include "soc/gpio_num.h"
#include "tasks/tasks.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// 50Hz on sec --> 100 zero crossings
// 10ms is average time between crossings 
// Impulse might be generated even 0.35ms before the actual zero crossing 
//
// Max switch period is 9.5ms; everything above should be treated as a constant signal 
//
// Timer is set up to activate 

// TIMERS ARE IN us
#define SAFE_ZONE_TIME          350
#define CONTROL_IMPULSE_TIME    100
#define GPIO_SEL_5 (1ULL << 5)
#define GPIO_SEL_18 (1ULL << 18)

bool * get_flag_zero_cossed(void);
void light_control_setup(void);
void light_toggle(unsigned int period);

#endif
