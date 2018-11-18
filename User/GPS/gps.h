#ifndef __GPS_H
#define __GPS_H	 

#include 	"config.h"

#define TIMETICKPERIOD  (250)

#define GPSDNTIMETICK  (60000/TIMETICKPERIOD)//(300000/TIMETICKPERIOD)   //300s
#define OffLineGPSnum 	3000
#define InLineGPSnum 	10
#define LineGPSLength 	16
#define GPSPaketnum 		100

#define GPS_RCCPeriph  (RCC_AHBPeriph_GPIOA)
#define GPS_PWUP_PORT 	(GPIOA)
#define GPS_PWUP_PIN 	(GPIO_Pin_4)


//UTCʱ����Ϣ
 typedef struct  
{										    
 	u8 year;	//���
	u8 month;	//�·�
	u8 date;	//����
	u8 hour; 	//Сʱ
	u8 min; 	//����
	u8 sec; 	//����
}nmea_utc_time;
//NMEA 0183 Э����������ݴ�Žṹ��
 typedef struct
{
	nmea_utc_time utc;			//UTCʱ��
	u32 latitude;				//γ�� ������100000��,ʵ��Ҫ����100000
	u8 nshemi;					//��γ/��γ,N:��γ;S:��γ				  
	u32 longitude;			    //���� ������100000��,ʵ��Ҫ����100000
	u8 ewhemi;					//����/����,E:����;W:����
 	u8 possl[12];				//���ڶ�λ�����Ǳ��
	u16 Course;               //����
	int altitude;			 	//���θ߶�,�Ŵ���10��,ʵ�ʳ���10.��λ:0.1m	 
	u16 speed;					//��������,�Ŵ���1000��,ʵ�ʳ���10.��λ:0.001����/Сʱ	 
}nmea_msg; 

extern nmea_msg gpsx;
extern u8 Gps_sendok;
extern u16 gpsoutputset;
extern u8 GPS_Valid_flag;
extern u16 rtc_updata_timeout;
extern u8 rtc_updata_flag;
extern u8 GPS_disp_flag;	//gps��ͼ����ʾ��־λ
extern u8 GPS_disp_tick;

u8 NMEA_Comma_Pos(u8 *buf,u8 cx);
u32 NMEA_Pow(u8 m,u8 n);
u32 NMEA_Str2num(u8 *buf,u8*dx);
u8 NMEA_GPGGA_Analysis(nmea_msg *gpsx,u8 *buf);
u8 NMEA_GPRMC_Analysis(nmea_msg *gpsx,u8 *buf);
void Gps_IO_Config(void);
void GPS_PWUP(void);
void GPS_PWOFF(void);
void GPS_SW(u8 s);
void GpsTimerHandler(void);
void Gps_Handler(void);
void TemporaryCloseInterrupt(void);
void TemporaryOpenInterrupt(void);
void gprsIN_GpsStore(u8 * buf);
void gprsOFF_GpsStore(nmea_msg *gps);
void gprsOFF_GpsSendQuick(void);
void GpsLocationOperate(nmea_msg *gps,u8* Serverindex,u16 mode,u8 link);

#endif  

 



