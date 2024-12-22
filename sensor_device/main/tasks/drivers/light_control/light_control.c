#include "light_control.h"
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

// 50Hz on sec --> 100 zero crossings
// 10ms is average time between crossings 
// impulse might be generated even 0,35ms before actual zero crossing 
//
// max switch period is 9,5ms and everything above should be treated as constant signal 
//
// timer is set up to activate 

// TIMERS ARE IN us
#define SAFE_ZONE_TIME          350
#define CONTROL_IMPULSE_TIME    100

static const char *TAG_LIGHT = "LIGHT CONTROL";

static unsigned int *light_period;


static esp_timer_handle_t light_period_timer = NULL;
static esp_timer_handle_t control_impulse_timer = NULL;


static void light_period_timer_cb(void *arg)
{
    gpio_set_level(GPIO_NUM_18, 1);
    esp_timer_start_once(control_impulse_timer, CONTROL_IMPULSE_TIME);
}

static void control_impulse_timer_cb(void *arg)
{
    gpio_set_level(GPIO_NUM_18, 0);
}


static void timer_setup(void)
{
    const esp_timer_create_args_t light_period_args = {
        .callback = &light_period_timer_cb,
        .name = "Light period timer",
        .dispatch_method = ESP_TIMER_ISR
    };

    const esp_timer_create_args_t control_impulse_args = {
        .callback = &control_impulse_timer_cb,
        .name = "Control impulse timer",
        .dispatch_method = ESP_TIMER_ISR
    };

    ESP_ERROR_CHECK(esp_timer_create(&light_period_args, &light_period_timer));
    ESP_ERROR_CHECK(esp_timer_create(&control_impulse_args, &control_impulse_timer));
}


void IRAM_ATTR interrupt_handler(void *p)
{
    if (*light_period <= 9500)
        esp_timer_start_once(light_period_timer, (*light_period + SAFE_ZONE_TIME));
}


void light_control_setup(void)
{
    const gpio_config_t ctrl_config = {
        GPIO_SEL_18,
        GPIO_MODE_OUTPUT,
        GPIO_PULLUP_DISABLE,
        GPIO_PULLDOWN_DISABLE,
        GPIO_INTR_DISABLE
    };

    gpio_config(&ctrl_config);
    gpio_set_level(GPIO_NUM_18, 0);
    ESP_LOGI(TAG_LIGHT, "Light control initialized");


    timer_setup();
    ESP_LOGI(TAG_LIGHT, "Timers initialized");

    light_period = get_light_period();

    const gpio_config_t zero_config = {
        GPIO_SEL_5,
        GPIO_MODE_INPUT,
        GPIO_PULLUP_DISABLE,
        GPIO_PULLDOWN_DISABLE,
        GPIO_INTR_POSEDGE
    };
    gpio_config(&zero_config);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_NUM_5, interrupt_handler, NULL);
    ESP_LOGI(TAG_LIGHT, "Zero-cross detection initialized");
}
