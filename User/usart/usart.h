#ifndef __USART_H
#define __USART_H

#include "stm32l1xx.h"
#include <stdio.h>

// 串口1-USART1	//GSM模块
#define  USART1_TX_GPIO_PORT       GPIOA   
#define  USART1_TX_GPIO_PIN        GPIO_Pin_9
#define  USART1_RX_GPIO_PORT       GPIOA
#define  USART1_RX_GPIO_PIN        GPIO_Pin_10

 //串口2-USART2	//GPS模块
#define  USART2_TX_GPIO_PORT       GPIOA   
#define  USART2_TX_GPIO_PIN        GPIO_Pin_2
#define  USART2_RX_GPIO_PORT       GPIOA
#define  USART2_RX_GPIO_PIN        GPIO_Pin_3

 //串口3-USART3
#define  USART3_TX_GPIO_PORT       GPIOB   
#define  USART3_TX_GPIO_PIN        GPIO_Pin_10
#define  USART3_RX_GPIO_PORT       GPIOB
#define  USART3_RX_GPIO_PIN        GPIO_Pin_11

#define UART0_MODE 'A' //A:ALL; M:GPRSOUT;S:SMALL

#define	GPRS_COM		1
#define  GPS_COM   	2
#define 	DEBUG_COM   3

void USART1_Config(u32 BAUDRATE);
void USART2_Config(u32 BAUDRATE);
void USART3_Config(u32 BAUDRATE);
void Usart_SendByte( USART_TypeDef * pUSARTx, uint8_t ch);
unsigned char Usart_ReceivByte(USART_TypeDef * pUSARTx);
void Usart_SendArray( USART_TypeDef * pUSARTx, uint8_t *array, uint16_t num);
void Usart_SendString( USART_TypeDef * pUSARTx, char *str);
void Usart_SendHalfWord( USART_TypeDef * pUSARTx, uint16_t ch);
void UARTWrite(u8 *Data, u16 NByte,u8 port);
void UARTWriteData(u8 *Data, u16 NByte,u8 port);
void SendData2TestCOM(u8 *Save_Data,u16 lenth,u8 comID);
#endif /* __USART_H*/

