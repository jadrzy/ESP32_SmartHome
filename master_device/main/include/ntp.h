#ifndef NTP_HEADER
#define NTP_HEADER

#include <time.h>

void time_sync_cb(struct timeval *tv);
void my_sntp_init(void);
void my_sntp_deinit(void);



#endif
