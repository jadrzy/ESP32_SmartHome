idf_component_register(
    SRCS 
        "src/main.c"
        "src/task.c"
        "src/components.c"
        "src/data.c"
        "src/wifi.c"
        "src/nvs.c"
        "src/button.c"
        "src/led.c"
        "src/ntp.c"
        "extlib/cJSON.c"
    INCLUDE_DIRS 
        "."
)
#    EMBED_TXTFILES
#        "serial/serial_number.txt")
