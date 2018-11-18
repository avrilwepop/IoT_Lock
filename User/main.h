#ifndef __MAIN_H
#define __MAIN_H

#include "stm32l1xx.h"
#include "bsp_ctr.h"
#include "usart.h"
#include "timer.h"
#include "key.h"
#include "lcd.h"
#include "systick.h"
#include "lp_mode.h"
#include "Rfid.h"
#include "i2c.h"

#define TASKS_MAX           					5
#define DEBUG_MODE           					1
#define NEED_GPS           						1
#define NEED_GPRS           					1

//�������нṹ��
typedef struct _TASK_COMPONENTS
{
    __IO uint32_t Run;                 	//�������б�־  		Run    0-������  1-����
    __IO uint32_t Timer;               	//�������м��ʱ��
    __IO uint32_t ItvTimer;            	//��װ��
    void (*TaskHook)(void);    				//Ҫ���е�������
}TASK_COMPONENTS;

void TaskReMarks(void);
void TaskProcess(void);
void Peripheral_Init(void);
#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
