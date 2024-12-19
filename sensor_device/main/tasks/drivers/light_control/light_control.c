#include "light_control.h"
#include "driver/gpio.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "hal/gpio_types.h"
#include "soc/gpio_num.h"

// 50Hz on sec --> 100 zero crossings
// 10ms is average time between crossings 
// impulse might be generated even 0,35ms before actual zero crossing 
//
// max switch period is 9,5ms and everything above should be treated as constant signal 
//
// timer is set up to activate 

static const char *TAG_LIGHT = "LIGHT CONTROL";

static bool flag_zero_crossed = 0;

bool * get_flag_zero_cossed(void)
{
    return &flag_zero_crossed;
}


static void zero_crossing(void)
{
    flag_zero_crossed = 1;
}

void IRAM_ATTR interrupt_handler(void *p)
{
    zero_crossing();
}

static void zero_crossing_setup(void)
{
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

    zero_crossing_setup();
}

void light_toggle(unsigned int period)
{
    if (period != 0)
    {
        gpio_set_level(GPIO_NUM_18, 1);
        ESP_LOGI(TAG_LIGHT, "ON");
        if (period != 10)
        {
            vTaskDelay(period);
            gpio_set_level(GPIO_NUM_18, 0);
            ESP_LOGI(TAG_LIGHT, "OFF");
        }
    }
}
