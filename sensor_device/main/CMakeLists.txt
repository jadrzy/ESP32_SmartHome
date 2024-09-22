idf_component_register(
  SRCS 
    "device.c" 
    "drivers/i2c/i2c.c" 
    "drivers/light_sensor/light_sensor.c"
    "drivers/light_sensor/calculate_lux.c"
  
  INCLUDE_DIRS 
    "."

)
