#!/bin/sh

# Function to check if a file exists
check_file() {
    if [ ! -f "$1" ]; then
        printf "Error: File not found: $1\n"
        exit 1
    fi
}

printf '\nSourcing framework...\n'
source ~/Documents/esp/esp-idf/export.sh

printf '\nCreating build...\n'
idf.py build

idf.py flash-erase

printf '\nFlashing build...\n'
idf.py flash 

# Check if the CSV file exists before making binary partitions
CSV_FILE="partitions/serial.csv"
BIN_FILE="partitions/serial.bin"

check_file "$CSV_FILE"

printf '\nMaking binary partitions...\n'
~/Documents/esp/esp-idf/components/nvs_flash/nvs_partition_generator/nvs_partition_gen.py generate "$CSV_FILE" "$BIN_FILE" 12288 

# Check if the binary partition file exists before flashing
check_file "$BIN_FILE"

printf '\nFlashing partitions...\n'
~/Documents/esp/esp-idf/components/partition_table/parttool.py write_partition --partition-name=serial --input "$BIN_FILE"

printf '\nDone.\n'
