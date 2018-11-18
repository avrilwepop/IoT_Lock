#ifndef __I2C_H
#define __I2C_H

#include "stm32l1xx.h"

#define I2C_WR	0		/* 写控制bit */
#define I2C_RD	1		/* 读控制bit */

/* 定义I2C eeprom */
#define I2C_GPIO_PORT	GPIOA										/* GPIO端口 */
#define I2C_RCC_PORT 	RCC_APB2Periph_GPIOA		/* GPIO端口时钟 */
#define I2C_SCL_PIN		GPIO_Pin_7							/* 连接到SCL时钟线的GPIO */
#define I2C_SDA_PIN		GPIO_Pin_6							/* 连接到SDA数据线的GPIO */
	

#define I2C_SCL_1  GPIO_SetBits(I2C_GPIO_PORT, I2C_SCL_PIN)		/* SCL = 1 */
#define I2C_SCL_0  GPIO_ResetBits(I2C_GPIO_PORT, I2C_SCL_PIN)		/* SCL = 0 */
	
#define I2C_SDA_1  GPIO_SetBits(I2C_GPIO_PORT, I2C_SDA_PIN)		/* SDA = 1 */
#define I2C_SDA_0  GPIO_ResetBits(I2C_GPIO_PORT, I2C_SDA_PIN)		/* SDA = 0 */
	
#define I2C_SDA_READ  GPIO_ReadInputDataBit(I2C_GPIO_PORT, I2C_SDA_PIN)	/* 读SDA口线状态 */

void I2C_Delay(void);
void I2C_GPIO_Config(void);
void I2C_Start(void);
void I2C_Stop(void);
void I2C_SendByte(uint8_t _ucByte);
uint8_t I2C_ReadByte(void);
uint8_t I2C_WaitAck(void);
void I2C_Ack(void);
void I2C_NAck(void);
uint8_t I2C_CheckDevice(uint8_t _Address);


#endif

