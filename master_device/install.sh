#!/bin/sh

printf '\nSourcing framework...\n'
source ~/Documents/esp/esp-idf/export.sh

printf '\nCreating build...\n'
idf.py build

printf '\nFlashing build...\n'
idf.py flash 

printf '\nMaking binary partitions...\n'
~/Documents/esp/esp-idf/components/nvs_flash/nvs_partition_generator/nvs_partition_gen.py "partitions/serial.csv" "partitions/serial.bin" 12288

printf '\nFlashing partitions...\n'
~/Documents/esp/esp-idf/components/partition_table/parttool.py write_partition --partition-name=serial --input "partitions/serial.bin"

printf '\nDone.\n'
