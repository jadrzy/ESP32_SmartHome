#ifndef NTP_HEADER
#define NTP_HEADER

#include <time.h>

// #define MAX_SNTP_SYNCH_TIME (2 * 60 * 60 * 1000)    // 2 hours
#define MAX_SNTP_SYNCH_TIME (60 * 1000)    // 30 seconds 

void time_sync_cb(struct timeval *tv);
void my_sntp_init(void);
void my_sntp_deinit(void);



#endif
