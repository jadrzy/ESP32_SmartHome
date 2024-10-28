#include "components/components.h"
#include "components/nvs/nvs.h"
#include "components/data/data.h" 
#include "components/wifi/wifi.h"


#include <freertos/projdefs.h>
#include <freertos/task.h>
#include <freertos/FreeRTOSConfig.h>
#include <freertos/FreeRTOS.h>
#include <freertos/idf_additions.h>
#include <freertos/semphr.h>
#include <stdint.h>


#ifndef TASK_HEADER
#define TASK_HEADER

#define WIFI_REINIT_INTERVAL (5000 / portTICK_PERIOD_MS)


typedef struct {
    TaskHandle_t wifi_setup; 
} my_task_handles_t;



#endif
