#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include "stm32l1xx.h"
#include <stdio.h>
#include "key.h"

//按键操作的状态
#define sNULL 	0
#define sInput 	1
#define sResult 2
#define sPress 	3 //长按关锁状态
#define sAlarm  4 //提示报警状态

typedef struct
{
	u8 hour;
	u8 min;
	u8 sec;
	u8 year;
	u8 month;
	u8 date;
}_s_Time;

extern _s_Time sys_Time;

//LCD显示中文结构体，大小：固定16x16
typedef struct _CN_TO_DISPLAY
{
	u8 Index;										//索引号
	char* str;                 				//要显示的字符
	u8 start_pos; 								//第一个字在数组中的位置
	u8 lenth; 										//长度
}CN_To_Display;

extern u16 Rx1Num;
extern u16 Rx2Num;
extern u16 Rx3Num;
extern u8 Rx1Buf[];
extern u8 Rx2Buf[];
extern u8 Rx3Buf[];
extern u8 LCD_BL_timeout;
extern u8 lcd_disp_flag;
extern u8 delay1stick;

extern eKey_sta key_state;
extern u8 volatile  rcv_usart1_str_flag;		//串口数据接收启动标记
extern u8 volatile  rcv_usart1_end_flag;		//串口数据接收结束标记
extern u8 volatile  rcv_usart1_over_count;	//数据接收时间溢出标记
/*usart2 send start and end flags & send dwell time counter */
extern u8 volatile  rcv_usart2_str_flag;		//串口数据接收启动标记
extern u8 volatile  rcv_usart2_end_flag;		//串口数据接收结束标记
extern u8 volatile  rcv_usart2_over_count;	//数据接收时间溢出标记
/*usart3 send start and end flags & send dwell time counter */
extern u8 volatile  rcv_usart3_str_flag;		//串口数据接收启动标记
extern u8 volatile  rcv_usart3_end_flag;		//串口数据接收结束标记
extern u8 volatile  rcv_usart3_over_count;		//数据接收时间溢出标记

void Do_Key(void);
void DO_Display(void);
void DO_USART1_GPRS(void);
void DO_USART2_GPS(void);
void DO_USART3_PC(void);
void DO_LOWPOWER_MODE(void);
void Update_Task(void);

#endif
