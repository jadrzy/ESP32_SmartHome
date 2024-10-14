idf_component_register(
  SRCS 
    "device.c" 
    "nvs/nvs.c"
    "tasks/tasks.c"
    "tasks/data/data.c"
    "tasks/drivers/i2c/i2c.c" 
    "tasks/drivers/light_sensor/light_sensor.c"
    "tasks/drivers/light_sensor/calculate_lux.c"
    "tasks/drivers/temp_hum_sensor/temp_hum_sensor.c"
    "tasks/drivers/pressure_sensor/pressure_sensor.c"
  
  INCLUDE_DIRS 
    "."

)
