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


//UTC时间信息
 typedef struct  
{										    
 	u8 year;	//年份
	u8 month;	//月份
	u8 date;	//日期
	u8 hour; 	//小时
	u8 min; 	//分钟
	u8 sec; 	//秒钟
}nmea_utc_time;
//NMEA 0183 协议解析后数据存放结构体
 typedef struct
{
	nmea_utc_time utc;			//UTC时间
	u32 latitude;				//纬度 分扩大100000倍,实际要除以100000
	u8 nshemi;					//北纬/南纬,N:北纬;S:南纬				  
	u32 longitude;			    //经度 分扩大100000倍,实际要除以100000
	u8 ewhemi;					//东经/西经,E:东经;W:西经
 	u8 possl[12];				//用于定位的卫星编号
	u16 Course;               //航向
	int altitude;			 	//海拔高度,放大了10倍,实际除以10.单位:0.1m	 
	u16 speed;					//地面速率,放大了1000倍,实际除以10.单位:0.001公里/小时	 
}nmea_msg; 

extern nmea_msg gpsx;
extern u8 Gps_sendok;
extern u16 gpsoutputset;
extern u8 GPS_Valid_flag;
extern u16 rtc_updata_timeout;
extern u8 rtc_updata_flag;
extern u8 GPS_disp_flag;	//gps的图标显示标志位
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

 



