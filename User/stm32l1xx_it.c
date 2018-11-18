#include "stm32l1xx_it.h"
#include "main.h"
#include "rfid.h"
#include "ttlm.h"
#include "gprs.h" 
#include "gps.h" 

extern u16 Rx1Num;
extern u16 Rx2Num;
extern u16 Rx3Num;
extern u8 Rx1Buf[];
extern u8 Rx2Buf[];
extern u8 Rx3Buf[];
extern u8 LCD_BL_timeout;
extern u8 KEY_reset_timeout;
/*usart1 send start and end flags & send dwell time counter */
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

extern eKey_sta key_state;
extern u8 isitCkey;

void NMI_Handler(void)
{}

void HardFault_Handler(void)
{
	NVIC_SystemReset();
	while (1)
  {
		printf("HardFault Error!\r\n");
		SysTick_Delay_Ms(1000);
  }
}

void MemManage_Handler(void)
{
  while (1)
  {
  }
}

void BusFault_Handler(void)
{
  while (1)
  {
  }
}

void UsageFault_Handler(void)
{
  while (1)
  {
  }
}

void SVC_Handler(void)
{}

void DebugMon_Handler(void)
{}

void PendSV_Handler(void)
{}

void SysTick_Handler(void)
{}

/*
MCU定时250ms唤醒一次
*/	
void RTC_WKUP_IRQHandler(void)
{
  if(RTC_GetITStatus(RTC_IT_WUT) != RESET)
  {
    RTC_ClearITPendingBit(RTC_IT_WUT);
    EXTI_ClearITPendingBit(EXTI_Line20);

		LockTicker++;
		Tick_250ms++;
		
		if(LCD_BL_timeout>0)
			LCD_BL_timeout--;
		if(KEY_LP_flag>0)
			KEY_LP_flag--;
		
		if(KEY_reset_timeout>0)
			KEY_reset_timeout--;
		
		if(gpsoutputset>0)
			gpsoutputset++;
		
		if((isitCkey>1)&&(key_state==KEY_RELEASE))
			isitCkey--;
		else
			isitCkey=0;
  } 
}

/*******************************************************************************
* Function Name  : EXTI15_10_IRQHandler
* Description    : DR引脚上升沿中断处理函数
* Input          : NONE.
* Return         : NONE.
*******************************************************************************/
void EXTI9_5_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line9)!= RESET)
	{
		RF_tick=20;	//20*250ms=5s
		RF_LP_flag=0;
		ReadRxData();
	}
	EXTI_ClearITPendingBit(EXTI_Line9);  //清除EXTI0线路挂起位
	EXTI_ClearFlag(EXTI_Line9);          //清楚EXTI9线路挂起标志位
}

//串口中断服务函数，接受GPRS数据
void USART1_IRQHandler(void)
{
	if(USART_GetITStatus(USART1, USART_IT_RXNE)==SET)		   	//判断，一旦接收到数据
	{
		USART_ClearITPendingBit(USART1,USART_IT_RXNE);		   	//清除中断标志位

		rcv_usart1_str_flag=1;					               				//串口数据接收标志置1
		rcv_usart1_over_count=0;			                 				//每来一字节溢出计数标志清0

		Rx1Buf[Rx1Num]= USART_ReceiveData(USART1);
		Rx1Num++;     //记录串口1收到的字节数
	}
	/*溢出-如果发生溢出需要先读SR,再读DR寄存器 则可清除不断入中断的问题*/
	if(USART_GetFlagStatus(USART1,USART_FLAG_ORE)==SET)
	{
		USART_ClearFlag(USART1,USART_FLAG_ORE);
		USART_ReceiveData(USART1);
	}
}

//串口中断服务函数，接受GPS数据
void USART2_IRQHandler(void)
{
	if(USART_GetITStatus(USART2, USART_IT_RXNE)==SET)		   //判断，一旦接收到数据
	{
		USART_ClearITPendingBit(USART2,USART_IT_RXNE);		   //清除中断标志位

		rcv_usart2_str_flag=1;					               //串口数据接收标志置1
		rcv_usart2_over_count=0;			                 //每来一字节，等待超时计数标志清0
		
		if((Rx2Num==0)&&(USART_ReceiveData(USART2)!='$'))
		{
			Rx2Num=0;
		}
		else
		{
			Rx2Buf[Rx2Num] = USART_ReceiveData(USART2);
			Rx2Num++;     //记录串口2收到的字节数
			if(Rx2Num >= RX2_BUF_SIZE)
				Rx2Num = 0;
		}
	}
	/*溢出-如果发生溢出需要先读SR,再读DR寄存器 则可清除不断入中断的问题*/
	if(USART_GetFlagStatus(USART2,USART_FLAG_ORE)==SET)
	{
		USART_ClearFlag(USART2,USART_FLAG_ORE);
		USART_ReceiveData(USART2);
	}
}

//串口中断服务函数，接受上位机数据
void USART3_IRQHandler(void)
{
	if(USART_GetITStatus(USART3, USART_IT_RXNE)==SET)		   //判断，一旦接收到数据
	{
		USART_ClearITPendingBit(USART3,USART_IT_RXNE);		   //清除中断标志位

		rcv_usart3_str_flag=1;					               //串口数据接收标志置1
		rcv_usart3_over_count=0;			                 //每来一字节，等待超时计数标志清0

		Rx3Buf[Rx3Num] = USART_ReceiveData(USART3);
		Rx3Num++;     //记录串口2收到的字节数
	}
	/*溢出-如果发生溢出需要先读SR,再读DR寄存器 则可清除不断入中断的问题*/
	if(USART_GetFlagStatus(USART3,USART_FLAG_ORE)==SET)
	{
		USART_ClearFlag(USART3,USART_FLAG_ORE);
		USART_ReceiveData(USART3);
	}
}

	/*tim2*/
void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		
		if(rcv_usart1_str_flag)					      //接收到了串口数据
		{
			if(rcv_usart1_over_count < 10)
				rcv_usart1_over_count++;			    //每一个数据接收完了最多等待10ms
			else
			{
				rcv_usart1_over_count = 0;
				rcv_usart1_str_flag=0;            //超过10ms，清标志位
				rcv_usart1_end_flag=1;				    //数据接收结束标志位置1
			}
		}
		if(rcv_usart2_str_flag)					      //接收到了串口数据
		{
			if(rcv_usart2_over_count < 10)
				rcv_usart2_over_count++;			    //每一个数据接收完了最多等待10ms
			else
			{
				rcv_usart2_str_flag=0;            //超过10ms，清标志位
				rcv_usart2_end_flag=1;				    //数据接收结束标志位置1
			}
		}
		if(rcv_usart3_str_flag)					      //接收到了串口数据
		{
			if(rcv_usart3_over_count < 10)
				rcv_usart3_over_count++;			    //每一个数据接收完了最多等待10ms
			else
			{
				rcv_usart3_str_flag=0;            //超过10ms，清标志位
				rcv_usart3_end_flag=1;				    //数据接收结束标志位置1
			}
		}
		if((key_state>KEY_NULL)&&(key_state<KEY_PRESS-1))
			key_state++;//有按键接触的状态，且没有判断按下的状态 自增
		
		//任务标志处理
    TaskReMarks();
  }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
