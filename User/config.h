#ifndef __CONFIG_H
#define __CONFIG_H

#include "stm32l1xx.h"
#include "systick.h"
#include "usart.h"
#include "bsp_ctr.h"

#define EN_IRQ          __enable_irq();     //系统开全局中断  
#define DIS_IRQ         __disable_irq();    //系统关全局中断
/*********************************************************************************************************/
/*********************************************************************************************************/
//各参数在缓存BUF里面的位置
#define LOCK_SN_POS							0//锁SN号
//锁参数存储字段
#define HEARTBEAT_GAP_POS				0																							//终端心跳发送间隔
#define TCP_TIMEOUT_POS					HEARTBEAT_GAP_POS+sizeof_heartbeat_gap				//TCP消息应答超时时间
#define GPS_GAP_SLEEP_POS				TCP_TIMEOUT_POS+sizeof_TCP_timeout						//休眠时汇报时间间隔
#define GPS_GAP_ACTIVE_POS				GPS_GAP_SLEEP_POS+sizeof_gps_gap_sleep				//缺省汇报时间间隔
#define ARC_CORNER_POS						GPS_GAP_ACTIVE_POS+sizeof_gps_gap_active			//拐点补传角度
#define GNSS_MODE_POS						ARC_CORNER_POS+sizeof_arc_corner							//GNSS定位模式
#define SERVER1_ADDR_LEN_POS			GNSS_MODE_POS+sizeof_gnss_mode								//服务器1地址长度
#define SERVER2_ADDR_LEN_POS			SERVER1_ADDR_LEN_POS+sizeof_server1_addr_len	//服务器2地址长度
#define SERVER3_ADDR_LEN_POS			SERVER2_ADDR_LEN_POS+sizeof_server2_addr_len	//服务器3地址长度
#define SERVER1_PORT_POS					SERVER3_ADDR_LEN_POS+sizeof_server3_addr_len	//服务器1端口号
#define SERVER2_PORT_POS					SERVER1_PORT_POS+sizeof_server1_port					//服务器2端口号
#define SERVER3_PORT_POS					SERVER2_PORT_POS+sizeof_server2_port					//服务器3端口号

#define SERVER1_ADDR_POS					//服务器1地址，IP或域名
#define SERVER2_ADDR_POS					//服务器2地址，IP或域名
#define SERVER3_ADDR_POS					//服务器3地址，IP或域名

//锁操作记录存储字段
#define OPERATION_TIME_POS				0																							//操作时间
#define OPERATION_TYPE_POS				OPERATION_TIME+sizeof_operate_time						//操作类型

//定位信息存储字段
#define ALARM_FLAG_POS						0																							//报警标志
#define LATITUDE_POS							ALARM_FLAG_POS+sizeof_alarm_flag							//纬度
#define LONGITUDE_POS						LATITUDE_POS+sizeof_latitude									//经度
#define GMT8_TIME_POS						LONGITUDE_POS+sizeof_longitude								//时间

/*********************************************************************************************************/
/***************************************EEPROM存储地址映射*************************************************/
/*********************************************************************************************************/
#define LOCKSN_ADDR1							128		//SN1号在EEPROM里面的地址
#define LOCKSN_ADDR2							134		//SN2号在EEPROM里面的地址
#define LOCK_PARA_ADDR						256		//锁参数在EEPROM里面的首地址
#define LOCK_RECORD_START_ADDR		384		//操作记录在EEPROM里面的起始地址 384开始

#define LOCK_RECORD_LEN					8			//锁操作记录的长度，8字节
#define LOCATE_INFO_LEN					16		//一条定位信息的长度，16字节









/*********************************************************************************************************/
/*********************************************************************************************************/

typedef enum
{
	//锁SN号
	sizeof_lock_sn=6,
	//设置锁信息
	sizeof_heartbeat_gap=4,
	sizeof_TCP_timeout=4,
	sizeof_gps_gap_sleep=4,
	sizeof_gps_gap_active=4,
	sizeof_arc_corner=1,
	sizeof_gnss_mode=1,
	sizeof_server1_addr_len=1,
	sizeof_server2_addr_len=1,
	sizeof_server3_addr_len=1,
	sizeof_server1_port=2,
	sizeof_server2_port=2,
	sizeof_server3_port=2,
	sizeof_reserve=10,
	//锁操作记录
	sizeof_operate_time=6,
	sizeof_operate_type=1,

	//GPS定位数据
	sizeof_alarm_flag=2,
	sizeof_latitude=4,
	sizeof_longitude=4,
	sizeof_gmt8_time=6,
}_sizeof_num;

typedef struct
{
	uint8_t lock_sn[6];
	
	uint32_t heartbeat_gap;
	uint32_t TCP_timeout;
	uint32_t gps_gap_sleep;
	uint32_t gps_gap_active;
	uint8_t arc_corner;
	uint8_t gnss_mode;
	uint8_t server1_addr_len;
	uint8_t server2_addr_len;
	uint8_t server3_addr_len;	
	uint32_t server1_port;
	uint32_t server2_port;
	uint32_t server3_port;
	char *server1_addr;
	char *server2_addr;
	char *server3_addr;
	uint8_t reserve[10];
}_set_lock_para;

typedef struct
{
	uint8_t operate_time[6];
	uint8_t operate_type;
}_lock_record;

typedef struct
{
	uint32_t alarm_flag;		//报警标志
	uint32_t locate_state;	//状态
	uint32_t latitude;			//纬度
	uint32_t longitude;			//经度
	uint16_t altitude;			//高度
	uint16_t speed;					//速度
	uint16_t direction;			//方向
	uint8_t gmt8_time[6];		//GMT+8时间
}_gps_para;

#endif /*__CONFIG_H*/

