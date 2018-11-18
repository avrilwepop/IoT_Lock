#include "lp_mode.h"
#include "common.h"
#include "lcd.h"
#include "rfid.h"
#include "I2C.h"

extern u8 lcd_disp_flag;

void RTC_Config(void)
{
	RTC_InitTypeDef RTC_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

	RTC_WakeUpCmd(DISABLE);

	PWR_ClearFlag(PWR_FLAG_WU);
	RTC_ClearFlag(RTC_FLAG_WUTF);

  /*!< Allow access to RTC */
  PWR_RTCAccessCmd(ENABLE);

  /*!< Reset RTC Domain */
  RCC_RTCResetCmd(ENABLE);
  RCC_RTCResetCmd(DISABLE);

  /*!< LSI Enable */
  RCC_LSICmd(ENABLE);

  /*!< Wait till LSI is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);

  /*!< RTC Clock Source Selection */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

  /*!< Enable the RTC Clock */
  RCC_RTCCLKCmd(ENABLE);

  /*!< Wait for RTC APB registers synchronisation */
  RTC_WaitForSynchro();

	/* Calendar Configuration这里RTC时钟是37KHz，如果不是，需要在system设置中更改 */
  RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
  RTC_InitStructure.RTC_SynchPrediv =  0x120; /* (37KHz / 128) - 1 = 0x120*/
  RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
  RTC_Init(&RTC_InitStructure);

  EXTI_ClearITPendingBit(EXTI_Line20);
  EXTI_InitStructure.EXTI_Line = EXTI_Line20;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

	  /* Enable the RTC Wakeup Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* RTC Wakeup Interrupt Generation: Clock Source: RTCDiv_16, Wakeup Time Base: 4s */
	RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div16);

  RTC_SetWakeUpCounter(0x242);	//0x242 = 578; 0.25s = (578+1)*(1/(37000/16))

	RTC_WakeUpCmd(ENABLE);

  /* Enable the Wakeup Interrupt */
  RTC_ITConfig(RTC_IT_WUT, ENABLE);
}

void RTC_OP(u8* Time,u8 mode)
{
	RTC_TimeTypeDef rtctime;
	RTC_DateTypeDef rtcdata;

	RTC_GetTime(RTC_Format_BCD, &rtctime);
	RTC_GetDate(RTC_Format_BCD, &rtcdata);
	switch(mode)
	{
		case RTC_GETTIME_BCD:
			Time[0] = rtcdata.RTC_Year;
			Time[1] = rtcdata.RTC_Month;
			Time[2] = rtcdata.RTC_Date;
			Time[3] = rtctime.RTC_Hours;
			Time[4] = rtctime.RTC_Minutes;
			Time[5] = rtctime.RTC_Seconds;
			break;
		case RTC_GETTIME_GB:
			Time[0] = 0x20;
			Time[1] = rtcdata.RTC_Year;
			Time[2] = rtcdata.RTC_Month;
			Time[3] = rtcdata.RTC_Date;
			Time[4] = rtctime.RTC_Hours;
			Time[5] = rtctime.RTC_Minutes;
			Time[6] = rtctime.RTC_Seconds;
			Time[7] = Do_XOR(Time,7);
			break;
		case RTC_SETTIME_BCD:
				/* Set the date: 周三 2018年1月3日 */
			rtcdata.RTC_Year = Time[0];
			rtcdata.RTC_Month = Time[1];
			rtcdata.RTC_Date = Time[2];
			rtcdata.RTC_WeekDay = 0x01;
			RTC_SetDate(RTC_Format_BCD, &rtcdata);//注意设置格式

			/* Set the time to 01h 00mn 00s AM */
			if(Time[3]>=0x12)
				rtctime.RTC_H12 = RTC_H12_PM;
			else
				rtctime.RTC_H12 = RTC_H12_AM;
			rtctime.RTC_Hours   = Time[3];
			rtctime.RTC_Minutes = Time[4];
			rtctime.RTC_Seconds = Time[5];
			RTC_SetTime(RTC_Format_BCD, &rtctime); //注意设置格式
			break;
		default:
			break;
	}
}

static void Set_TO_HSI(void)
{
	RCC->CR |= ((uint32_t)RCC_CR_HSION);;
	while ((RCC->CR & RCC_CR_HSIRDY) == RESET);
	RCC->CR |= RCC_CR_PLLON;
	while ((RCC->CR & RCC_CR_PLLRDY)== RESET);
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
	while (RCC_GetSYSCLKSource() != 0x0C);
}

static void PeriphClk_OFF(void)
{
	SetOFFMode();
	
	/* 关闭ADC */
	ADC_Cmd(ADC1, DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, DISABLE);

	/* 关闭TIM2 */
	TIM_ITConfig(TIM2,  TIM_IT_Update, DISABLE);
	TIM_Cmd(TIM2, DISABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, DISABLE);

	/* 降低usart1时钟 */
	RCC_APB2PeriphClockLPModeCmd(RCC_APB2Periph_USART1,ENABLE);

	/* 关闭GPIO时钟 */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA|RCC_AHBPeriph_GPIOB|RCC_AHBPeriph_GPIOC|RCC_AHBPeriph_GPIOD, DISABLE);

	/* 关闭SYSTCIK */
	NVIC_DisableIRQ(SysTick_IRQn);
}

static void PeriphClk_ON(void)
{
	/* 打开ADC */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	ADC_Cmd(ADC1, ENABLE);

	/* 打开SYSTCIK */
	//NVIC_EnableIRQ(SysTick_IRQn);
	
	/* 打开TIM2 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	TIM_ITConfig(TIM2,  TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM2, ENABLE);

	/* 降低usart1时钟 */
	RCC_APB2PeriphClockLPModeCmd(RCC_APB2Periph_USART1,ENABLE);

	/* 打开GPIO时钟 */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA|RCC_AHBPeriph_GPIOB|RCC_AHBPeriph_GPIOC|RCC_AHBPeriph_GPIOD, ENABLE);

	RFM_Ini(LockCHcfg);
}

void LPMODE_OP(void)
{
	/****关闭LCD显示*****/
	LCD_BL_OFF;
	lcd_disp_flag=0;
	LCD_Disp_Off();

	//printf("mcuzzz\r\n");

	/* 关闭外设时钟 */
	PeriphClk_OFF();

	/* Enter Sleep Mode */
	PWR_EnterSleepMode(PWR_Regulator_LowPower, PWR_SLEEPEntry_WFI);

//	/* Enter Stop Mode */
//	PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
//
//	/* Enter Standby Mode */
//	PWR_EnterSTANDBYMode();

	/* 打开外设时钟 */
	PeriphClk_ON();
	printf("mcuggg\r\n");
}



