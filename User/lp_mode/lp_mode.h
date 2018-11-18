#ifndef __LP_MODE_H
#define __LP_MODE_H

#include "stm32l1xx.h"

#define RTC_GETTIME_BCD  1
#define RTC_GETTIME_GB   2
#define RTC_SETTIME_BCD  3

void RTC_Config(void);
void RTC_OP(u8* Time,u8 mode);
void LPMODE_OP(void);
#endif

