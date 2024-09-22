idf_component_register(
  SRCS 
    "device.c" 
    "tasks/tasks.c"
    "tasks/data/data.c"
    "tasks/drivers/i2c/i2c.c" 
    "tasks/drivers/light_sensor/light_sensor.c"
    "tasks/drivers/light_sensor/calculate_lux.c"
  
  INCLUDE_DIRS 
    "."

)
