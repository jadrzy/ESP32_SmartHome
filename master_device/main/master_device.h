#ifndef MASTER_DEVICE_HEADER
#define MASTER_DEVICE_HEADER

#define TAG "Master Device"

// Constants
#define SERIAL_NUMBER_SIZE 13
#define NUMBER_OF_DEVICES 10
#define WIFI_CREDENTIALS_SIZE 33 

typedef struct {
    char serial[SERIAL_NUMBER_SIZE];
    uint64_t mac;
} device_t;

#endif
