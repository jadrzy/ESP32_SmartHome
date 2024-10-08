#include "master_device.h"
#include "nvs/nvs.h"
#include "nvs_flash.h"

void app_main(void)
{
    init_nvs();
    operation();
}
