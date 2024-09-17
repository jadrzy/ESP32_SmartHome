idf_component_register(SRCS "device.c" 
                            "i2c/i2c.c" 
                            "i2c/light_sensor/calculate_lux.c" 
                            "i2c/light_sensor/light_sensor.c"
                            "i2c/temp_hum_sensor/temp_hum_sensor.c"
                            "i2c/pressure_sensor/pressure_sensor.c"
                    INCLUDE_DIRS ".")
