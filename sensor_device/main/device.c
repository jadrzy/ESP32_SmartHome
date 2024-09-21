#include "device.h"


static struct i2c i2c = {
    .init_status = 0
};

static struct data data;

void app_main(void)
{
    ESP_LOGI("TAG", "app_started");
    i2c_init(&i2c);
}
