#include "include/button.h"
#include "esp_log.h"

static const char* TAG_BUTTON = "BUTTON DRIVER";

static TimerHandle_t debouncer_timer = NULL;
static TimerHandle_t button_pressed_timer = NULL;
static int button_state;
static bool press_timer_running = false;

void button_init(void)
{
    ESP_LOGI(TAG_BUTTON, "Button init");

    const gpio_config_t config = {
                    GPIO_SEL_23,
                    GPIO_MODE_INPUT,
                    GPIO_PULLUP_ENABLE,
                    GPIO_PULLDOWN_DISABLE,
                    GPIO_INTR_ANYEDGE
    };
    gpio_config(&config);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_NUM_23, button_handler, NULL);

    button_state = gpio_get_level(GPIO_NUM_23);

    // debouncer setup
    debouncer_timer = xTimerCreate(
                            "Debouncer timer",
                            pdMS_TO_TICKS(10),
                            pdFALSE,
                            (void *)0,
                            debouncer_callback_timer);

    // pressed and hold button setup
    debouncer_timer = xTimerCreate(
                            "Button press timer",
                            pdMS_TO_TICKS(PRESS_TIME),
                            pdFALSE,
                            (void *)0,
                            button_press_callback_timer);
}

void IRAM_ATTR button_handler(void *p)
{
    xTimerStart(debouncer_timer, 5); 
}


void debouncer_callback_timer(TimerHandle_t xTimer)
{
    int new_state = gpio_get_level(GPIO_NUM_23);

    if (!press_timer_running)
    {
        // CHECK IF BUTTON WAS PRESSED OR IF IT WAS INTERFERENCE
        if(button_state - new_state == 1)
        {
            xTimerStart(button_pressed_timer, 5); 
            press_timer_running = true;
        }
        button_state = new_state;
    }
    else
    {
        if(button_state != new_state)
        {
            xTimerStop(button_pressed_timer, 5); 
            press_timer_running = false;
        }
    }
}


void button_press_callback_timer(TimerHandle_t xTimer)
{
    press_timer_running = false;
    start_setup_mode();
}
