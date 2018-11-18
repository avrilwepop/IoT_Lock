#ifndef __BSP_CTR_H
#define __BSP_CTR_H

#include "stm32l1xx.h"
#include "systick.h"

/*�弶���ƶ˿�GPIO��ʼ��*/

//GPS GOTOPģ����ƶ˿�
//GPS_EN    	PA4
#define GPS_EN_GPIO_PIN				GPIO_Pin_4
#define GPS_EN_GPIO_PORT			GPIOA
#define GPS_POWER_ON					GPS_EN_GPIO_PORT ->ODR |=  GPS_EN_GPIO_PIN
#define GPS_POWER_OFF			  	GPS_EN_GPIO_PORT ->ODR &= ~GPS_EN_GPIO_PIN

//WTDG���ƶ˿�
//WTDG_EN    	PA1
//WTDG_IO    	PA0 
#define WTDG_EN_PIN						GPIO_Pin_1
#define WTDG_EN_PORT  				GPIOA

#define WTDG_IO_PIN  					GPIO_Pin_0
#define WTDG_IO_PORT  				GPIOA

#define WTDG_DISABLE					WTDG_EN_PORT ->ODR |=  WTDG_EN_PIN
#define WTDG_ENABLE			  		WTDG_EN_PORT ->ODR &= ~WTDG_EN_PIN
#define WTDG_IO_HIGH					WTDG_IO_PORT ->ODR |=  WTDG_IO_PIN
#define WTDG_IO_LOW			  		WTDG_IO_PORT ->ODR &= ~WTDG_IO_PIN

#define FEED_WTDG 			  		{WTDG_IO_HIGH;SysTick_Delay_Us(1);	WTDG_IO_LOW;}

//��������ź�
#define MOTOR_OPEN  					{GPIOB ->ODR &= ~GPIO_Pin_8;	GPIOB ->ODR |= GPIO_Pin_9;}
#define MOTOR_CLOSE  					{GPIOB ->ODR |= GPIO_Pin_8;	GPIOB ->ODR &= ~GPIO_Pin_9;}
#define MOTOR_STOP 						{GPIOB ->ODR &= ~GPIO_Pin_8; GPIOB ->ODR &= ~GPIO_Pin_9;}

//��������������
//#define HALL_Check_A						GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_14)//��߹̶�����  ���˴�1�����˱պ�0
#define HALL_Check_B					GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_15)//�ұ߲��̶����� ���˰γ�1�����˲���0
#define HALL_Check_C					GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_15)//�м����Ǳ���  ʩ�⵽λ1����⵽λ0

#define POWER_CTR_ON					GPIO_SetBits(GPIOA,GPIO_Pin_5)
#define POWER_CTR_OFF					GPIO_ResetBits(GPIOA,GPIO_Pin_5)

void BSP_CTR_Config(void);
void Motor_IO_Config(void);

void ADC1_Init (void);
u16 Get_Adc(void); 
u16 DO_BatVol(void); 
void Peripheral_Init(void);

#endif

