#ifndef __SYSTICK_H
#define __SYSTICK_H

#include "stm32l1xx.h"

#define sysclk 32000000 //32M

void SysTick_Delay_Us( __IO uint32_t us);
void SysTick_Delay_Ms( __IO uint32_t ms);

#endif /*__SYSTICK_H*/

