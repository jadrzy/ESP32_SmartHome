#include "data.h"


static struct data sensor_data = {0}; 

double get_lux(void)
{
    return sensor_data.lux;
}

double get_temperature(void)
{
    return sensor_data.temperature;
}

double get_humidity(void)
{
    return sensor_data.humidity;
}

double get_pressure(void)
{
    return sensor_data.pressure;
}


void set_lux(double value)
{
    sensor_data.lux = value; 
}

void set_temperature(double value)
{
    sensor_data.temperature = value;
}

void set_humidity(double value)
{
    sensor_data.humidity = value;
}

void set_pressure(double value)
{
    sensor_data.pressure = value;
}
