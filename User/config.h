#ifndef __CONFIG_H
#define __CONFIG_H

#include "stm32l1xx.h"
#include "systick.h"
#include "usart.h"
#include "bsp_ctr.h"

#define EN_IRQ          __enable_irq();     //ϵͳ��ȫ���ж�  
#define DIS_IRQ         __disable_irq();    //ϵͳ��ȫ���ж�
/*********************************************************************************************************/
/*********************************************************************************************************/
//�������ڻ���BUF�����λ��
#define LOCK_SN_POS							0//��SN��
//�������洢�ֶ�
#define HEARTBEAT_GAP_POS				0																							//�ն��������ͼ��
#define TCP_TIMEOUT_POS					HEARTBEAT_GAP_POS+sizeof_heartbeat_gap				//TCP��ϢӦ��ʱʱ��
#define GPS_GAP_SLEEP_POS				TCP_TIMEOUT_POS+sizeof_TCP_timeout						//����ʱ�㱨ʱ����
#define GPS_GAP_ACTIVE_POS				GPS_GAP_SLEEP_POS+sizeof_gps_gap_sleep				//ȱʡ�㱨ʱ����
#define ARC_CORNER_POS						GPS_GAP_ACTIVE_POS+sizeof_gps_gap_active			//�յ㲹���Ƕ�
#define GNSS_MODE_POS						ARC_CORNER_POS+sizeof_arc_corner							//GNSS��λģʽ
#define SERVER1_ADDR_LEN_POS			GNSS_MODE_POS+sizeof_gnss_mode								//������1��ַ����
#define SERVER2_ADDR_LEN_POS			SERVER1_ADDR_LEN_POS+sizeof_server1_addr_len	//������2��ַ����
#define SERVER3_ADDR_LEN_POS			SERVER2_ADDR_LEN_POS+sizeof_server2_addr_len	//������3��ַ����
#define SERVER1_PORT_POS					SERVER3_ADDR_LEN_POS+sizeof_server3_addr_len	//������1�˿ں�
#define SERVER2_PORT_POS					SERVER1_PORT_POS+sizeof_server1_port					//������2�˿ں�
#define SERVER3_PORT_POS					SERVER2_PORT_POS+sizeof_server2_port					//������3�˿ں�

#define SERVER1_ADDR_POS					//������1��ַ��IP������
#define SERVER2_ADDR_POS					//������2��ַ��IP������
#define SERVER3_ADDR_POS					//������3��ַ��IP������

//��������¼�洢�ֶ�
#define OPERATION_TIME_POS				0																							//����ʱ��
#define OPERATION_TYPE_POS				OPERATION_TIME+sizeof_operate_time						//��������

//��λ��Ϣ�洢�ֶ�
#define ALARM_FLAG_POS						0																							//������־
#define LATITUDE_POS							ALARM_FLAG_POS+sizeof_alarm_flag							//γ��
#define LONGITUDE_POS						LATITUDE_POS+sizeof_latitude									//����
#define GMT8_TIME_POS						LONGITUDE_POS+sizeof_longitude								//ʱ��

/*********************************************************************************************************/
/***************************************EEPROM�洢��ַӳ��*************************************************/
/*********************************************************************************************************/
#define LOCKSN_ADDR1							128		//SN1����EEPROM����ĵ�ַ
#define LOCKSN_ADDR2							134		//SN2����EEPROM����ĵ�ַ
#define LOCK_PARA_ADDR						256		//��������EEPROM������׵�ַ
#define LOCK_RECORD_START_ADDR		384		//������¼��EEPROM�������ʼ��ַ 384��ʼ

#define LOCK_RECORD_LEN					8			//��������¼�ĳ��ȣ�8�ֽ�
#define LOCATE_INFO_LEN					16		//һ����λ��Ϣ�ĳ��ȣ�16�ֽ�









/*********************************************************************************************************/
/*********************************************************************************************************/

typedef enum
{
	//��SN��
	sizeof_lock_sn=6,
	//��������Ϣ
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
	//��������¼
	sizeof_operate_time=6,
	sizeof_operate_type=1,

	//GPS��λ����
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
	uint32_t alarm_flag;		//������־
	uint32_t locate_state;	//״̬
	uint32_t latitude;			//γ��
	uint32_t longitude;			//����
	uint16_t altitude;			//�߶�
	uint16_t speed;					//�ٶ�
	uint16_t direction;			//����
	uint8_t gmt8_time[6];		//GMT+8ʱ��
}_gps_para;

#endif /*__CONFIG_H*/

