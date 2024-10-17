#include "master_device.h"
#include "esp_log.h"
#include "components/nvs/nvs.h"
#include "nvs_flash.h"
#include <stdio.h>
#include <string.h>

void app_main(void)
{
    init_nvs_partitions();
    run();
}
