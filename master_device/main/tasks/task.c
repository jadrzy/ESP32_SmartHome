#include "task.h"
#include "freertos/idf_additions.h"
#include "portmacro.h"

static const char TAG_TASK[] = "TASK";

static my_task_handles_t task_handles;
static int wifi_setup_status = true;


void wifi_setup_task(void *p)
{
    bool wifi_initialized = false;

    while (wifi_initialized != true){
        wifi_initialized = wifi_init(p);

        vTaskDelay(WIFI_REINIT_INTERVAL);
    }

    vTaskDelete(NULL);
}


void initialize_wifi_setup_task(bool *wifi_setup_mode)
{
    BaseType_t err;
    err = xTaskCreate(wifi_setup_task, "Wifi setup task", 6144, wifi_setup_mode, 3, &task_handles.wifi_setup);
    if (err == pdPASS)
        ESP_LOGI(TAG_TASK, "Wifi setup task initialized with mode = %d", *wifi_setup_mode);
    else 
        ESP_LOGE(TAG_TASK, "Wifi setup task error with mode = %d", *wifi_setup_mode);
}
