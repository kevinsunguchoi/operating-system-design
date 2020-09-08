/* 
 * rtc.h - Defines used in interactions with the rtc interrupt
 * controller
 */

#ifndef _RTC_H
#define _RTC_H

#define RTC_PORT       0x70
#define RTC_DATA       0x71
#define RTC_A          0x8A
#define RTC_B          0x8B
#define RTC_C          0x8C
#define MAX_INT_FREQ   32768   //maximum interrupt frequency by rtc 2^15
#define K_MAX_INT_FREQ 1024    //maximum interrupt frequency by kernel 2^10 
#define K_MAX_INT_RATE 6       //maximum interrupt rate by kernel is 6

//initialize rtc
void rtc_init();
//set the rtc rate
// void rtc_set_rate(unsigned char rate);

int32_t rtc_open(const uint8_t* filename);
int32_t rtc_close(int32_t fd);
int32_t rtc_read(int32_t fd, uint8_t* buf, int32_t nbytes);
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);

#endif
