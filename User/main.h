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

//程序运行结构体
typedef struct _TASK_COMPONENTS
{
    __IO uint32_t Run;                 	//程序运行标志  		Run    0-不运行  1-运行
    __IO uint32_t Timer;               	//程序运行间隔时间
    __IO uint32_t ItvTimer;            	//重装载
    void (*TaskHook)(void);    				//要运行的任务函数
}TASK_COMPONENTS;

void TaskReMarks(void);
void TaskProcess(void);
void Peripheral_Init(void);
#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
