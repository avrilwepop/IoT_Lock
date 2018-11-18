/**
  ******************************************************************************
  * @file    eePROM.c
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   I2C EEPROM(AT24C02)应用函数
  ******************************************************************************
  */ 
#include "eeprom.h"
#include <stdio.h>
#include "I2C.h"

/*
*********************************************************************************************************
*	函 数 名: ee_CheckOk
*	功能说明: 判断串行EERPOM是否正常
*	形    参：无
*	返 回 值: 1 表示正常， 0 表示不正常
*********************************************************************************************************
*/
uint8_t ee_CheckOk(void)
{
	if (I2C_CheckDevice(EEPROM_DEV_ADDR) == 0)
	{
		return 1;
	}
	else
	{
		/* 失败后，切记发送I2C总线停止信号 */
		I2C_Stop();		
		return 0;
	}
}

/*
*********************************************************************************************************
*	函 数 名: ee_ReadByte
*	功能说明: 
*	形    参：_usAddress : 起始地址
*	返 回 值: 读取得8位数据
*********************************************************************************************************
*/
uint8_t ee_ReadByte( uint16_t _usAddress )
{
	uint8_t temp=0;
	
	/* 采用串行EEPROM随即读取指令序列，连续读取2个字节 */
	
	/* 第1步：发起I2C总线启动信号 */
	I2C_Start();
	
	/* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
	I2C_SendByte(EEPROM_DEV_ADDR | I2C_WR);	/* 此处是写指令 */
	 
	/* 第3步：等待ACK */
	if (I2C_WaitAck() != 0)
	{
		goto cmd_fail;	/* EEPROM器件无应答 */
	}

	/* 第4步：发送字节地址，24C02只有256字节，因此1个字节就够了，如果是24C04以上，那么此处需要连发多个地址 */
	I2C_SendByte(_usAddress/256);//写入高地址	
	if (I2C_WaitAck() != 0)
	{
		goto cmd_fail;	/* EEPROM器件无应答 */
	}
	
	I2C_SendByte(_usAddress%256);//写入低地址
	/* 第5步：等待ACK */
	if (I2C_WaitAck() != 0)
	{
		goto cmd_fail;	/* EEPROM器件无应答 */
	}
	
	/* 第6步：重新启动I2C总线。前面的代码的目的向EEPROM传送地址，下面开始读取数据 */
	I2C_Start();
	
	/* 第7步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
	I2C_SendByte(EEPROM_DEV_ADDR | I2C_RD);	/* 此处是读指令 */
	
	/* 第8步：发送ACK */
	if (I2C_WaitAck() != 0)
	{
		goto cmd_fail;	/* EEPROM器件无应答 */
	}	
	/* 第9步：读取数据 */
	temp = I2C_ReadByte();	/* 读1个字节 需要发送Ack， 最后一个字节不需要Ack，发Nack*/

	I2C_NAck();	

	/* 发送I2C总线停止信号 */
	I2C_Stop();
	return temp;	/* 执行成功 */

cmd_fail: /* 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备 */
	/* 发送I2C总线停止信号 */
	I2C_Stop();
	return 0;
}

/*
*********************************************************************************************************
*	函 数 名: ee_ReadWord
*	功能说明: 
*	形    参：_usAddress : 起始地址
*	返 回 值: 读取得16位数据
*********************************************************************************************************
*/
uint16_t ee_ReadWord( uint16_t _usAddress )
{
	uint16_t temp=0;
	
	/* 采用串行EEPROM随即读取指令序列，连续读取2个字节 */
	
	/* 第1步：发起I2C总线启动信号 */
	I2C_Start();
	
	/* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
	I2C_SendByte(EEPROM_DEV_ADDR | I2C_WR);	/* 此处是写指令 */
	 
	/* 第3步：等待ACK */
	if (I2C_WaitAck() != 0)
	{
		goto cmd_fail;	/* EEPROM器件无应答 */
	}

	/* 第4步：发送字节地址，24C02只有256字节，因此1个字节就够了，如果是24C04以上，那么此处需要连发多个地址 */
	I2C_SendByte(_usAddress/256);//写入高地址	
	if (I2C_WaitAck() != 0)
	{
		goto cmd_fail;	/* EEPROM器件无应答 */
	}
	
	I2C_SendByte(_usAddress%256);//写入低地址
	/* 第5步：等待ACK */
	if (I2C_WaitAck() != 0)
	{
		goto cmd_fail;	/* EEPROM器件无应答 */
	}
	
	/* 第6步：重新启动I2C总线。前面的代码的目的向EEPROM传送地址，下面开始读取数据 */
	I2C_Start();
	
	/* 第7步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
	I2C_SendByte(EEPROM_DEV_ADDR | I2C_RD);	/* 此处是读指令 */
	
	/* 第8步：发送ACK */
	if (I2C_WaitAck() != 0)
	{
		goto cmd_fail;	/* EEPROM器件无应答 */
	}	
	/* 第9步：读取数据 */
	temp = I2C_ReadByte();	/* 读1个字节 需要发送Ack， 最后一个字节不需要Ack，发Nack*/
	I2C_Ack();	/* 中间字节读完后，CPU产生ACK信号(驱动SDA = 0) */
	
	temp <<= 8; //空出低8位
	temp  |= I2C_ReadByte();/* 最后1个字节读完后，CPU产生NACK信号(驱动SDA = 1) */
	I2C_NAck();	

	/* 发送I2C总线停止信号 */
	I2C_Stop();
	return temp;	/* 执行成功 */

cmd_fail: /* 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备 */
	/* 发送I2C总线停止信号 */
	I2C_Stop();
	return 0;
}

/*
*********************************************************************************************************
*	函 数 名: ee_ReadBytes
*	功能说明: 从串行EEPROM指定地址处开始读取若干数据
*	形    参：_usAddress : 起始地址
*			 _usSize : 数据长度，单位为字节
*			 _pReadBuf : 存放读到的数据的缓冲区指针
*	返 回 值: 0 表示失败，1表示成功
*********************************************************************************************************
*/
uint8_t ee_ReadBytes(uint8_t *_pReadBuf, uint16_t _usAddress, uint16_t _usSize)
{
	uint16_t i;
	
	/* 采用串行EEPROM随即读取指令序列，连续读取若干字节 */
	
	/* 第1步：发起I2C总线启动信号 */
	I2C_Start();
	
	/* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
	I2C_SendByte(EEPROM_DEV_ADDR | I2C_WR);	/* 此处是写指令 */
	 
	/* 第3步：等待ACK */
	if (I2C_WaitAck() != 0)
	{
		goto cmd_fail;	/* EEPROM器件无应答 */
	}

	/* 第4步：发送字节地址，24C02只有256字节，因此1个字节就够了，如果是24C04以上，那么此处需要连发多个地址 */
	I2C_SendByte(_usAddress/256);//写入高地址	
	if (I2C_WaitAck() != 0)
	{
		goto cmd_fail;	/* EEPROM器件无应答 */
	}
	
	I2C_SendByte(_usAddress%256);//写入低地址
	/* 第5步：等待ACK */
	if (I2C_WaitAck() != 0)
	{
		goto cmd_fail;	/* EEPROM器件无应答 */
	}
	
	/* 第6步：重新启动I2C总线。前面的代码的目的向EEPROM传送地址，下面开始读取数据 */
	I2C_Start();
	
	/* 第7步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
	I2C_SendByte(EEPROM_DEV_ADDR | I2C_RD);	/* 此处是读指令 */
	
	/* 第8步：发送ACK */
	if (I2C_WaitAck() != 0)
	{
		goto cmd_fail;	/* EEPROM器件无应答 */
	}	
	
	/* 第9步：循环读取数据 */
	for (i = 0; i < _usSize; i++)
	{
		_pReadBuf[i] = I2C_ReadByte();	/* 读1个字节 */
		
		/* 每读完1个字节后，需要发送Ack， 最后一个字节不需要Ack，发Nack */
		if (i != _usSize - 1)
		{
			I2C_Ack();	/* 中间字节读完后，CPU产生ACK信号(驱动SDA = 0) */
		}
		else
		{
			I2C_NAck();	/* 最后1个字节读完后，CPU产生NACK信号(驱动SDA = 1) */
		}
	}
	/* 发送I2C总线停止信号 */
	I2C_Stop();
	return 1;	/* 执行成功 */

cmd_fail: /* 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备 */
	/* 发送I2C总线停止信号 */
	I2C_Stop();
	return 0;
}

/*
*********************************************************************************************************
*	函 数 名: ee_WriteByte
*	功能说明: 向串行EEPROM指定地址写入一个字节的数据
*	形    参：_usAddress : 写入的地址
*			 DataToWrite : 要写入的数据
*	返 回 值: 0 表示失败，1表示成功
*********************************************************************************************************
*/
uint8_t ee_WriteByte(uint8_t DataToWrite, uint16_t _usAddress )
{
	/* 第1步：发起I2C总线启动信号 */
	I2C_Start();

	/* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
	I2C_SendByte(EEPROM_DEV_ADDR | I2C_WR);	/* 此处是写指令 */
				
	/* 第3步：发送一个时钟，判断器件是否正确应答 */
	I2C_WaitAck();

	/* 第4步：发送字节地址，此处需要连发2个地址 */
	I2C_SendByte(_usAddress/256);//写入高地址
	if (I2C_WaitAck() != 0)
	{
		goto cmd_fail;	/* EEPROM器件无应答 */
	}
			
	I2C_SendByte(_usAddress%256);//写入低地址
	/* 第5步：等待ACK */
	if (I2C_WaitAck() != 0)
	{
		goto cmd_fail;	/* EEPROM器件无应答 */
	}
	
	/* 第6步：开始写入数据 */
	I2C_SendByte( DataToWrite);
	
	/* 第7步：发送ACK */
	if (I2C_WaitAck() != 0)
	{
		goto cmd_fail;	/* EEPROM器件无应答 */
	}

	/* 命令执行成功，发送I2C总线停止信号 */
	I2C_Stop();
	ee_Delay(0x0FFFF);//byte write模式效率低，需要等待一段时间,一般不超过10ms，让数据写完
	return 1;

cmd_fail: /* 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备 */
	/* 发送I2C总线停止信号 */
	I2C_Stop();
	return 0;
}
/*
*********************************************************************************************************
*	函 数 名: ee_WriteBytes
*	功能说明: 向串行EEPROM指定地址写入若干数据，采用页写操作提高写入效率
*	形    参：_usAddress : 起始地址
*			 _usSize : 数据长度，单位为字节
*			 _pWriteBuf : 存放读到的数据的缓冲区指针
*	返 回 值: 0 表示失败，1表示成功
*********************************************************************************************************
*/
uint8_t ee_WriteBytes(uint8_t *_pWriteBuf, uint16_t _usAddress, uint16_t _usSize)
{
	uint16_t i,m;
	uint16_t usAddr;
	
	/* 
		写串行EEPROM不像读操作可以连续读取很多字节，每次写操作只能在同一个page。
		对于24xx02，page size = 8
		简单的处理方法为：按字节写操作模式，没写1个字节，都发送地址
		为了提高连续写的效率: 本函数采用page wirte操作。
	*/

	usAddr = _usAddress;	
	for (i = 0; i < _usSize; i++)
	{
		/* 当发送第1个字节或是页面首地址时，需要重新发起启动信号和地址 */
		if (  (i == 0) || ( (usAddr % (EEPROM_PAGE_SIZE)) == 0 )  )
		{
			/*　第０步：发停止信号，启动内部写操作　*/
			I2C_Stop();
			
			/* 通过检查器件应答的方式，判断内部写操作是否完成, 一般小于 10ms 			
				CLK频率为200KHz时，查询次数为30次左右
			*/
			for (m = 0; m < 1000; m++)
			{				
				/* 第1步：发起I2C总线启动信号 */
				I2C_Start();
				
				/* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
				I2C_SendByte(EEPROM_DEV_ADDR | I2C_WR);	/* 此处是写指令 */
				
				/* 第3步：发送一个时钟，判断器件是否正确应答 */
				if (I2C_WaitAck() == 0)
				{
					break;
				}
			}
			if (m  == 1000)
			{
				goto cmd_fail;	/* EEPROM器件写超时 */
			}
		
			/* 第4步：发送字节地址，此处需要连发2个地址 */
			
			I2C_SendByte(usAddr/256);//写入高地址
			if (I2C_WaitAck() != 0)
			{
				goto cmd_fail;	/* EEPROM器件无应答 */
			}
			
			I2C_SendByte(usAddr%256);//写入低地址
			/* 第5步：等待ACK */
			if (I2C_WaitAck() != 0)
			{
				goto cmd_fail;	/* EEPROM器件无应答 */
			}
		}
	
		/* 第6步：开始写入数据 */
		I2C_SendByte(_pWriteBuf[i]);
	
		/* 第7步：发送ACK */
		if (I2C_WaitAck() != 0)
		{
			goto cmd_fail;	/* EEPROM器件无应答 */
		}

		usAddr++;	/* 地址增1 */		
	}
	
	/* 命令执行成功，发送I2C总线停止信号 */
	I2C_Stop();
	ee_Delay(0x0FFFF);//byte write模式效率低，需要等待一段时间,一般不超过10ms，让数据写完
	return 1;

cmd_fail: /* 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备 */
	/* 发送I2C总线停止信号 */
	I2C_Stop();
	return 0;
}

u8 EEPROM_OP(u8 *datbuf,u32 address,u16 num,u8 mode)
{    
	switch (mode)
	{
		case 2:
			ee_ReadBytes(datbuf,address,num);
			break;
		
		case 1:
			ee_WriteBytes(datbuf,address,num);
			break;
		
		default: 
			break;
	}
	return 1;
}

void ee_Erase(void)
{
	uint16_t i,j;
	uint8_t buf[EEPROM_PAGE_SIZE];
	
	/* 填充缓冲区 */
	for (i = 0; i < EEPROM_PAGE_SIZE; i++)
	{
		buf[i] = 0xFF;
	}
	for(j=0;j<512;j++)
	{
		ee_WriteBytes(buf, j*128, EEPROM_PAGE_SIZE);
	}
//	
//	/* 写EEPROM, 起始地址 = 0，数据长度为 128 */
//	if (ee_WriteBytes(buf, 0, EEPROM_PAGE_SIZE) == 0)
//	{
//		printf("擦除eeprom出错！\r\n");
//		return;
//	}
//	else
//	{
//		printf("擦除eeprom成功！\r\n");
//	}
}

/*--------------------------------------------------------------------------------------------------*/
static void ee_Delay(__IO uint32_t nCount)	 //简单的延时函数
{
	for(; nCount != 0; nCount--);
}

/*********************************************END OF FILE**********************/
