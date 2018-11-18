#ifndef __KEY_H
#define __KEY_H

#include "stm32l1xx.h"

#define Row1_GPIO_PIN  		GPIO_Pin_4
#define Row2_GPIO_PIN  		GPIO_Pin_5
#define Row3_GPIO_PIN  		GPIO_Pin_6

#define Col1_GPIO_PIN  		GPIO_Pin_0
#define Col2_GPIO_PIN  		GPIO_Pin_1
#define Col3_GPIO_PIN  		GPIO_Pin_2
#define Col4_GPIO_PIN  		GPIO_Pin_3

typedef enum 
{
	KEY_NULL=0,
	KEY_TOUCH=1,
	KEY_DEBOUNCE=3,	//ȥ����ʱ�䣬��λms
	KEY_PRESS=30,		//ȡֵ����ν����ȥ����ʱ������
	KEY_RELEASE=40,	//ȡֵ����ν����PRESS�����
}eKey_sta;

static void RowScan_GPIO_Config(void);
static void ColScan_GPIO_Config(void);
char Keyboard_Scan(void);
#endif /*__KEY_H*/

