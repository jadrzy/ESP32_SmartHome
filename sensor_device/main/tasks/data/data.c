#include "data.h"


static struct data sensor_data = {0}; 

uint32_t get_lux(void)
{
    return sensor_data.lux;
}


void set_lux(uint32_t value)
{
    sensor_data.lux = value; 
}
