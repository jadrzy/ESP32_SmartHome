#include "light_control.h"
#include "driver/gpio.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "hal/gpio_types.h"
#include "soc/gpio_num.h"
#include "tasks/tasks.h"

// 50Hz on sec --> 100 zero crossings
// 10ms is average time between crossings 
// impulse might be generated even 0,35ms before actual zero crossing 
//
// max switch period is 9,5ms and everything above should be treated as constant signal 
//
// timer is set up to activate 

static const char *TAG_LIGHT = "LIGHT CONTROL";

static TimerHandle_t timer = NULL;

static bool flag_zero_crossed = 0;

static unsigned int *light_period;
int light_periodd = 1;

// bool * get_flag_zero_cossed(void)
// {
//     return &flag_zero_crossed;
// }


static void zero_crossing(void)
{
    gpio_set_level(GPIO_NUM_18, 1);
    xTimerStart(timer, 0);
    // flag_zero_crossed = 1;
}

void IRAM_ATTR interrupt_handler(void *p)
{
    zero_crossing();
}

static void zero_crossing_setup(void)
{
    light_period = get_light_period();

    const gpio_config_t config = {
        GPIO_SEL_5,
        GPIO_MODE_INPUT,
        GPIO_PULLUP_DISABLE,
        GPIO_PULLDOWN_DISABLE,
        GPIO_INTR_POSEDGE
    };

    gpio_config(&config);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_NUM_5, interrupt_handler, NULL);

    ESP_LOGI(TAG_LIGHT, "Zero crossing detection initialized");

}

void timer_handle(TimerHandle_t xTimer)
{
    gpio_set_level(GPIO_NUM_18, 0);
}

void light_control_setup(void)
{
    const gpio_config_t config = {
        GPIO_SEL_18,
        GPIO_MODE_OUTPUT,
        GPIO_PULLUP_DISABLE,
        GPIO_PULLDOWN_DISABLE,
        GPIO_INTR_DISABLE
    };

    gpio_config(&config);
    gpio_set_level(GPIO_NUM_18, 0);
    ESP_LOGI(TAG_LIGHT, "Light control initialized");

    timer = xTimerCreate("Period timer", pdMS_TO_TICKS(light_periodd), pdFALSE, (void *)0, timer_handle);

    zero_crossing_setup();
}


// void light_toggle(unsigned int period)
// {
//     if (period != 0)
//     {
//         gpio_set_level(GPIO_NUM_18, 1);
//         ESP_LOGI(TAG_LIGHT, "ON");
//         if (period != 10)
//         {
//             vTaskDelay(period);
//             gpio_set_level(GPIO_NUM_18, 0);
//             ESP_LOGI(TAG_LIGHT, "OFF");
//         }
//     }
// }
