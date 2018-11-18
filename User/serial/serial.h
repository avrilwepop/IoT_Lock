#ifndef __SERIAL_H
#define __SERIAL_H

#include "stm32l1xx.h"
#include "usart.h"
#include "eeprom.h"

#define CMD_WRITE_SN								0x51		//写入SN号
#define CMD_WRITE_SN_ACK						0x52		//响应写入SN号
#define CMD_SET_LOCK     					0x53		//设置锁参数
#define CMD_QUERY_LOCK							0x54		//查询锁参数
#define CMD_SET_QUERY_ACK					0x55		//响应 设置、查询锁参数

typedef enum
{
  FRM_HEADD_ERROR = -1,
	FRM_LEN_ERROR = -2,
	FRM_CRC_ERROR = -3,
	WRITE_SN_OK = 1,
	SET_LOCK_OK = 2,
	QUERY_LOCK_OK = 3,
}_ack_status;

typedef enum
{
  RES_SUCCESS = 0,
	RES_COMP_FAIL = 1,
	RES_SN_ERROR = 2,
}_result_status;

int Serial3_RxCB(void);
void Serial3_TxED(void);
void Serial3_Write_SN(void);
void Serial3_SET_LOCK(void);
void Serial3_QUERY_LOCK(void);
#endif //__SERIAL_H

