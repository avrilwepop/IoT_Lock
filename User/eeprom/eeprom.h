#ifndef __EEPROM_H
#define __EEPROM_H

#include "stm32l1xx.h"

/* 
 * AT24C02 2kb = 2048bit = 2048/8 B = 256 BYTE
 * 32 pages of 8 bytes each
 
 * AT24C512 512kb = 524 288bit = 524 288/8 BYTE = 65536 BYTE = 64KB
 * 512 pages of 128 bytes each
 *
 * Device Address
 * 1 0 1 0  A2 A1 A0 R/W
 * 1 0 1 0  0  0  0   0   = 0XA0
 * 1 0 1 0  0  0  0   1   = 0XA1 
 */

/* AT24C01/02每页有8个字节 
 * AT24C04/08A/16A每页有16个字节 
 * AT24C512  每页有128个字节
 */
	

#define EEPROM_DEV_ADDR			0xA0		/* 24xx512的设备地址 */
#define EEPROM_PAGE_SIZE			128			  /* 24xx512的页面大小 */
#define EEPROM_SIZE				  65536			  /* 24xx512总容量 */

u8 EEPROM_OP(u8 *datbuf,u32 address,u16 num,u8 mode);
uint8_t	 ee_CheckOk(void);
uint8_t	 ee_ReadByte( uint16_t _usAddress );
uint16_t ee_ReadWord( uint16_t _usAddress );
uint8_t  ee_ReadBytes(uint8_t *_pReadBuf, uint16_t _usAddress, uint16_t _usSize);
uint8_t  ee_WriteByte(uint8_t DataToWrite, uint16_t _usAddress );
uint8_t  ee_WriteBytes(uint8_t *_pWriteBuf, uint16_t _usAddress, uint16_t _usSize);

void ee_Erase(void);
static void ee_Delay(__IO uint32_t nCount);	 //简单的延时函数


#endif /* __EEPROM_H */
