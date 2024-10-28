idf_component_register(
    SRCS 
        "master_device.c"
        "tasks/task.c"
        #"components/data/data.c"
        "components/wifi/wifi.c"
        "components/nvs/nvs.c"
        "components/components.c"
    INCLUDE_DIRS 
        "."
)
#    EMBED_TXTFILES
#        "serial/serial_number.txt")
