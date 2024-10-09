idf_component_register(
    SRCS 
        "master_device.c"
        # "tasks/task.c"
        # "tasks/data/data.c"
        # "tasks/wifi/wifi.c"
        "nvs/nvs.c"
    INCLUDE_DIRS 
        "."
)
#    EMBED_TXTFILES
#        "serial/serial_number.txt")
