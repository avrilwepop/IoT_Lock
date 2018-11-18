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
extern u8 volatile  rcv_usart1_str_flag;		//�������ݽ����������
extern u8 volatile  rcv_usart1_end_flag;		//�������ݽ��ս������			             
extern u8 volatile  rcv_usart1_over_count;	//���ݽ���ʱ��������
/*usart2 send start and end flags & send dwell time counter */
extern u8 volatile  rcv_usart2_str_flag;		//�������ݽ����������
extern u8 volatile  rcv_usart2_end_flag;		//�������ݽ��ս������			             
extern u8 volatile  rcv_usart2_over_count;	//���ݽ���ʱ��������
/*usart3 send start and end flags & send dwell time counter */
extern u8 volatile  rcv_usart3_str_flag;		//�������ݽ����������
extern u8 volatile  rcv_usart3_end_flag;		//�������ݽ��ս������			             
extern u8 volatile  rcv_usart3_over_count;		//���ݽ���ʱ��������

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
MCU��ʱ250ms����һ��
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
* Description    : DR�����������жϴ�����
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
	EXTI_ClearITPendingBit(EXTI_Line9);  //���EXTI0��·����λ
	EXTI_ClearFlag(EXTI_Line9);          //���EXTI9��·�����־λ
}

//�����жϷ�����������GPRS����
void USART1_IRQHandler(void)
{
	if(USART_GetITStatus(USART1, USART_IT_RXNE)==SET)		   	//�жϣ�һ�����յ�����
	{
		USART_ClearITPendingBit(USART1,USART_IT_RXNE);		   	//����жϱ�־λ

		rcv_usart1_str_flag=1;					               				//�������ݽ��ձ�־��1
		rcv_usart1_over_count=0;			                 				//ÿ��һ�ֽ����������־��0

		Rx1Buf[Rx1Num]= USART_ReceiveData(USART1);
		Rx1Num++;     //��¼����1�յ����ֽ���
	}
	/*���-������������Ҫ�ȶ�SR,�ٶ�DR�Ĵ��� �������������жϵ�����*/
	if(USART_GetFlagStatus(USART1,USART_FLAG_ORE)==SET)
	{
		USART_ClearFlag(USART1,USART_FLAG_ORE);
		USART_ReceiveData(USART1);
	}
}

//�����жϷ�����������GPS����
void USART2_IRQHandler(void)
{
	if(USART_GetITStatus(USART2, USART_IT_RXNE)==SET)		   //�жϣ�һ�����յ�����
	{
		USART_ClearITPendingBit(USART2,USART_IT_RXNE);		   //����жϱ�־λ

		rcv_usart2_str_flag=1;					               //�������ݽ��ձ�־��1
		rcv_usart2_over_count=0;			                 //ÿ��һ�ֽڣ��ȴ���ʱ������־��0
		
		if((Rx2Num==0)&&(USART_ReceiveData(USART2)!='$'))
		{
			Rx2Num=0;
		}
		else
		{
			Rx2Buf[Rx2Num] = USART_ReceiveData(USART2);
			Rx2Num++;     //��¼����2�յ����ֽ���
			if(Rx2Num >= RX2_BUF_SIZE)
				Rx2Num = 0;
		}
	}
	/*���-������������Ҫ�ȶ�SR,�ٶ�DR�Ĵ��� �������������жϵ�����*/
	if(USART_GetFlagStatus(USART2,USART_FLAG_ORE)==SET)
	{
		USART_ClearFlag(USART2,USART_FLAG_ORE);
		USART_ReceiveData(USART2);
	}
}

//�����жϷ�������������λ������
void USART3_IRQHandler(void)
{
	if(USART_GetITStatus(USART3, USART_IT_RXNE)==SET)		   //�жϣ�һ�����յ�����
	{
		USART_ClearITPendingBit(USART3,USART_IT_RXNE);		   //����жϱ�־λ

		rcv_usart3_str_flag=1;					               //�������ݽ��ձ�־��1
		rcv_usart3_over_count=0;			                 //ÿ��һ�ֽڣ��ȴ���ʱ������־��0

		Rx3Buf[Rx3Num] = USART_ReceiveData(USART3);
		Rx3Num++;     //��¼����2�յ����ֽ���
	}
	/*���-������������Ҫ�ȶ�SR,�ٶ�DR�Ĵ��� �������������жϵ�����*/
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
		
		if(rcv_usart1_str_flag)					      //���յ��˴�������
		{
			if(rcv_usart1_over_count < 10)
				rcv_usart1_over_count++;			    //ÿһ�����ݽ����������ȴ�10ms
			else
			{
				rcv_usart1_over_count = 0;
				rcv_usart1_str_flag=0;            //����10ms�����־λ
				rcv_usart1_end_flag=1;				    //���ݽ��ս�����־λ��1
			}
		}
		if(rcv_usart2_str_flag)					      //���յ��˴�������
		{
			if(rcv_usart2_over_count < 10)
				rcv_usart2_over_count++;			    //ÿһ�����ݽ����������ȴ�10ms
			else
			{
				rcv_usart2_str_flag=0;            //����10ms�����־λ
				rcv_usart2_end_flag=1;				    //���ݽ��ս�����־λ��1
			}
		}
		if(rcv_usart3_str_flag)					      //���յ��˴�������
		{
			if(rcv_usart3_over_count < 10)
				rcv_usart3_over_count++;			    //ÿһ�����ݽ����������ȴ�10ms
			else
			{
				rcv_usart3_str_flag=0;            //����10ms�����־λ
				rcv_usart3_end_flag=1;				    //���ݽ��ս�����־λ��1
			}
		}
		if((key_state>KEY_NULL)&&(key_state<KEY_PRESS-1))
			key_state++;//�а����Ӵ���״̬����û���жϰ��µ�״̬ ����
		
		//�����־����
    TaskReMarks();
  }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
