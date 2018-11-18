#ifndef __RFID_H
#define __RFID_H

#include 	"config.h"

#define WC			0x00			// Write configuration register command
#define RC			0x10 			// Read  configuration register command
#define WTP		0x20 			// Write TX Payload  command
#define RTP		0x21			// Read  TX Payload  command
#define WTA		0x22			// Write TX Address  command
#define RTA		0x23			// Read  TX Address  command
#define RRP		0x24			// Read  RX Payload  command

#define TRX_CE_H 					GPIO_SetBits(GPIOA,GPIO_Pin_11)
#define TRX_CE_L 					GPIO_ResetBits(GPIOA,GPIO_Pin_11)

#define TXEN_H						GPIO_SetBits(GPIOA,GPIO_Pin_12)
#define TXEN_L 						GPIO_ResetBits(GPIOA,GPIO_Pin_12)

#define CSN_H 						GPIO_SetBits(GPIOB,GPIO_Pin_14)
#define CSN_L							GPIO_ResetBits(GPIOB,GPIO_Pin_14)

#define RFPWR_UP_H 				GPIO_SetBits(GPIOA,GPIO_Pin_8)
#define RFPWR_UP_L 				GPIO_ResetBits(GPIOA,GPIO_Pin_8)

#define	SPI_RF_MOSI_H     GPIO_SetBits(GPIOC,GPIO_Pin_7)
#define	SPI_RF_MOSI_L    	GPIO_ResetBits(GPIOC,GPIO_Pin_7)

#define	SPI_RF_SCK_H      GPIO_SetBits(GPIOB,GPIO_Pin_15)
#define	SPI_RF_SCK_L      GPIO_ResetBits(GPIOB,GPIO_Pin_15)

#define	SPI_RF_MISO    		GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_8)

#define TT_MODE	0x5A
#define GB_MODE	0x66

#define RFCH0 95
#define RFCH4 115
#define RFCH5 120

//extern//////////////
extern u8 LockCHcfg[10];
extern u8 RFRxBuf[100];
extern u8 RF_tick;

void NRF905_IOinit(void);
void RFM_Ini(u8 * rfcfg);
void RFINT_en(void);
void RFINT_off(void);
void SetRxMode(void);
void SetTxMode(void);
void SetOFFMode(void);
void Set_StandBy(void);
void SPI_Send(u8 byte);
u8 SPI_Rcv(void);
void Config905(u8 *rfcfgbuf);
u16 RdRfBuf(u8 * buf);
void  ReadRxData(void);
void TxData(void);
void GB_LockSend(u8 * buf,u8 len);
void EXTI9_5_IRQHandler(void);
void GB_LockSend(u8 * buf,u8 len);
void RF_Process(void);
void CenterCmdOP(u8 * buf,u8 lenth,u8* Serverindex,u16 cmd,u8 op);
void GB_RFRxDataOP(u8 lenth);
void GB_RFACK(void);
u8 GB_RFTxBufGet(u8 rfcmd);
void GB_RFTxDatPacketSend(u8 cmd,u8 mode,u8 trycnt);
u8 NEW_RFTxBufGet(u8 rfcmd);
void NEW_RFTxDatPacketSend(u8 cmd,u8 mode,u8 trycnt);
u8 KEY_VN(u8 flag,u8 rrcmd);
void SetDynamicPassword(u8* sn,u32 count);

#endif

