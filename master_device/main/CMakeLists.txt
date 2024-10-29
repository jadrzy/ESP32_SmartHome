idf_component_register(
    SRCS 
        "src/main.c"
        "srctask.c"
        "src/data.c"
        "src/wifi.c"
        "src/nvs.c"
        "src/components.c"
    INCLUDE_DIRS 
        "."
)
#    EMBED_TXTFILES
#        "serial/serial_number.txt")
