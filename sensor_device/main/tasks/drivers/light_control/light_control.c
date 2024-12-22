#include "light_control.h"

static const char *TAG_LIGHT = "LIGHT CONTROL";

static unsigned int *light_period;

static esp_timer_handle_t light_period_timer = NULL;
static esp_timer_handle_t control_impulse_timer = NULL;

// Timer callback for light period
static void light_period_timer_cb(void *arg)
{
    gpio_set_level(GPIO_NUM_18, 1);
    esp_timer_start_once(control_impulse_timer, CONTROL_IMPULSE_TIME);
}

// Timer callback for control impulse
static void control_impulse_timer_cb(void *arg)
{
    gpio_set_level(GPIO_NUM_18, 0);
}

// Function to set up timers
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

// Interrupt handler for zero-cross detection
void IRAM_ATTR interrupt_handler(void *p)
{
    if (*light_period <= 9500)
    {
        esp_timer_start_once(light_period_timer, (*light_period + SAFE_ZONE_TIME));
    }
}

// Function to set up light control
void light_control_setup(void)
{
    // Configure control GPIO
    const gpio_config_t ctrl_config = {
        .pin_bit_mask = GPIO_SEL_18,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&ctrl_config);
    gpio_set_level(GPIO_NUM_18, 0);
    ESP_LOGI(TAG_LIGHT, "Light control initialized");

    // Initialize timers
    timer_setup();
    ESP_LOGI(TAG_LIGHT, "Timers initialized");

    // Get light period pointer
    light_period = get_light_period();

    // Configure zero-cross detection GPIO
    const gpio_config_t zero_config = {
        .pin_bit_mask = GPIO_SEL_5,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_POSEDGE
    };
    gpio_config(&zero_config);

    // Install ISR service and add handler
    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_NUM_5, interrupt_handler, NULL);
    ESP_LOGI(TAG_LIGHT, "Zero-cross detection initialized");
}
