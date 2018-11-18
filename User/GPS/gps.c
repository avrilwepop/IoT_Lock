#include "gps.h"
#include <math.h>
#include <stdlib.h>
#include "common.h"
#include "string.h"
#include "TTLM.H"
#include "rfid.h"
#include "gprs.h"
#include "config.h"
#include "crcencoder.h"
#include "BSprotocol.h"
#include "gbprotocol.h"
#include "global.h"
#include "lp_mode.h"

nmea_msg gpsx;
u8 Gps_sendok = 0;
u8 gpsaccuratetick = 0;
u32 gpsUpTimeTick=0;
u32 gpsdnTimeTick = 0;
u8 GPS_Valid_flag;
u8 InLineGPSbuf[InLineGPSnum][LineGPSLength+1];
u16 OffLineGPSindex;
u32 Latitude100000;
u32 Longitude100000;
u32 GGALatitude100000;
u32 GGALongitude100000;
u16 gpsoutputset=0;
u8 rtc_updata_flag=0;
u16 rtc_updata_timeout=0;
u8 GPS_disp_tick=0;	//gps的图标显示标志位
u8 GPS_disp_flag=0;	
/*******************************************************************************
* Function Name  : NMEA_Comma_Pos
* Description    : NMEA_Comma_Pos
* Input          :
* Return         :
*******************************************************************************/
u8 NMEA_Comma_Pos(u8 *buf,u8 cx)
{
	u8 *p=buf;

	while(cx)
	{
		if(*buf=='*'||*buf<' '||*buf>'z')return 0XFF;//遇到'*'或者非法字符,则不存在第cx个逗号
		if(*buf==',')cx--;
		buf++;
	}
	return buf-p;
}
/*******************************************************************************
* Function Name  : NMEA_Pow
* Description    : NMEA_Pow
* Input          :
* Return         :
*******************************************************************************/
u32 NMEA_Pow(u8 m,u8 n)
{
	u32 result=1;

	while(n--)result*=m;
	return result;
}
/*******************************************************************************
* Function Name  : NMEA_Str2num
* Description    : NMEA_Str2num
* Input          :
* Return         :
*******************************************************************************/
u32 NMEA_Str2num(u8 *buf,u8*dx)
{
	u8 *p=buf;
	u32 ires=0,fres=0;
	u8 ilen=0,flen=0,i;
	u8 mask=0;
	u32 res;

	while(1) //得到整数和小数的长度
	{
		if(*p=='-'){mask|=0X02;p++;}//是负数
		if(*p==','||(*p=='*'))break;//遇到结束了
		if(*p=='.'){mask|=0X01;p++;}//遇到小数点了
		else if(*p>'9'||(*p<'0'))	//有非法字符
		{
			ilen=0;
			flen=0;
			break;
		}
		if(mask&0X01)flen++;
		else ilen++;
		p++;
	}
	if(mask&0X02)buf++;	//去掉负号
	for(i=0;i<ilen;i++)	//得到整数部分数据
	{
		ires+=NMEA_Pow(10,ilen-1-i)*(buf[i]-'0');
	}
	if(flen>5)flen=5;	//最多取5位小数
	*dx=flen;	 		//小数点位数
	for(i=0;i<flen;i++)	//得到小数部分数据
	{
		fres+=NMEA_Pow(10,flen-1-i)*(buf[ilen+1+i]-'0');
	}
	res=ires*NMEA_Pow(10,flen)+fres;
	if(mask&0X02)res=-res;
	return res;
}
/*******************************************************************************
* Function Name  : NMEA_GPGGA_Analysis
* Description    : NMEA_GPGGA_Analysis
* Input          :
* Return         :
*******************************************************************************/
u8 NMEA_GPGGA_Analysis(nmea_msg *gpsx,u8 *buf)
{
	u8 *p1,dx;
	u8 posx;

	p1=(u8*)strstr((const char *)buf,"$GNGGA");

	if(p1 == NULL)
		return 0;

	posx=NMEA_Comma_Pos(p1,2);
	if(posx!=0XFF)
		GGALatitude100000 = NMEA_Str2num(p1+posx,&dx);
	else
		return 0;
	posx=NMEA_Comma_Pos(p1,4);
	if(posx!=0XFF)
		GGALongitude100000 = NMEA_Str2num(p1+posx,&dx);
	else
		return 0;
	posx=NMEA_Comma_Pos(p1,9);								//得到海拔高度
	if(posx!=0XFF)
		gpsx->altitude=NMEA_Str2num(p1+posx,&dx)/NMEA_Pow(10,dx);
	return 1;
}

/*******************************************************************************
* Function Name  : NMEA_GPRMC_Analysis
* Description    : NMEA_GPRMC_Analysis
* Input          :
* Return         :
*******************************************************************************/
u8 NMEA_GPRMC_Analysis(nmea_msg *gpsx,u8 *buf)
{
	u8 *p1,dx;
	u8 posx;
	u32 temp;
	float rs;

	p1=(u8*)strstr((const char *)buf,"GNRMC");//"$GNRMC",经常有&和GPRMC分开的情况,故只判断GPRMC.

	if(p1 == NULL)
		return 0;
	posx=NMEA_Comma_Pos(p1,1);								//得到UTC时间
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx);
		temp = temp/NMEA_Pow(10,dx);	 					//得到UTC时间,去掉ms
		gpsx->utc.hour=HexToBcd(temp/10000+8);
		gpsx->utc.min=HexToBcd((temp/100)%100);
		gpsx->utc.sec=HexToBcd(temp%100);
	}
	else
		return 0;

	posx=NMEA_Comma_Pos(p1,2);								//定位状态，A=有效定位，V=无效定位
	if(posx!=0XFF)
	{
		GPS_Valid_flag = ((*(p1+posx)== 'A')?(1):(0));
	}
	else
	{
		GPS_Valid_flag = 0;
	}
	GPS_disp_flag=GPS_Valid_flag;
	
	posx=NMEA_Comma_Pos(p1,3);								//得到纬度
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx);
		Latitude100000 = temp;
		gpsx->latitude=temp/NMEA_Pow(10,dx+2);	//得到°
		rs=temp%NMEA_Pow(10,dx+2);				//得到'
		gpsx->latitude =(u32)(gpsx->latitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60);//转换为°
	}
	else
		return 0;
	posx=NMEA_Comma_Pos(p1,4);								//南纬还是北纬
	if(posx!=0XFF)gpsx->nshemi=*(p1+posx);
 	posx=NMEA_Comma_Pos(p1,5);								//得到经度
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx);
		Longitude100000 = temp;
		gpsx->longitude=temp/NMEA_Pow(10,dx+2);	//得到°
		rs=temp%NMEA_Pow(10,dx+2);				//得到'
		gpsx->longitude=(u32)(gpsx->longitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60);//转换为°
	}
	else
		return 0;

	posx=NMEA_Comma_Pos(p1,6);								//东经还是西经
	if(posx!=0XFF)gpsx->ewhemi=*(p1+posx);
	posx=NMEA_Comma_Pos(p1,7);								//得到速度
	if(posx!=0XFF)
	{
		temp = NMEA_Str2num(p1+posx,&dx);
		temp = temp*1852/NMEA_Pow(10,dx)/100;        //节=    1.852km/h
		gpsx->speed = temp;                                 //单位  0.1km/h
	}
	posx=NMEA_Comma_Pos(p1,8);								//得到航向

	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx);
		gpsx->Course = temp/NMEA_Pow(10,dx);
	}
	posx=NMEA_Comma_Pos(p1,9);								//得到UTC日期
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx);		 				//得到UTC日期

		gpsx->utc.date=HexToBcd(temp/10000);
		gpsx->utc.month=HexToBcd((temp/100)%100);
		gpsx->utc.year= HexToBcd(temp%100);
	}
	else
		return 0;

	return 1;
}

/*******************************************************************************
* Function Name  : GPS_PWUP
* Description    : GPS_PWUP
* Input          :
* Return         :
*******************************************************************************/
void GPS_PWUP(void)
{
	GPIO_SetBits(GPS_PWUP_PORT,GPS_PWUP_PIN);
}

/*******************************************************************************
* Function Name  : GPS_PWOFF
* Description    : GPS_PWOFF
* Input          :
* Return         :
*******************************************************************************/
void GPS_PWOFF(void)
{
	GPIO_ResetBits(GPS_PWUP_PORT,GPS_PWUP_PIN);
}

/*******************************************************************************
* Function Name  : GPS_PWOFF
* Description    : GPS_PWOFF
* Input          :
* Return         :
*******************************************************************************/
void GPS_SW(u8 s)
{
	if(s)
	{
		GPS_PWUP();
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
		USART_Cmd(USART2, ENABLE);
		NVIC_EnableIRQ(USART2_IRQn);
		GPS_ON_Flag=1;
		GPS_OFF_Flag = 0;
		gpsoutputset=1;
	}
	else
	{
		GPS_PWOFF();
		GPS_ON_Flag=0;
		GPS_OFF_Flag = 1;
		USART_Cmd(USART2, DISABLE);
		NVIC_DisableIRQ(USART2_IRQn);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,DISABLE);
	}
}

/*******************************************************************************
* Function Name  : GpsTimerHandler
* Description    : 检查GPS定时有没有到
* Input          :
* Return         :
*******************************************************************************/
void GpsTimerHandler(void)
{
	rtc_updata_timeout++;
	if(rtc_updata_timeout>=14400)//14400*250ms=60分钟,更新下RTC时间
	{
		rtc_updata_flag=0;
		rtc_updata_timeout=0;
	}
	if((GPS_ON_Flag)&&(!GPS_OFF_Flag))
	{
		gpsdnTimeTick ++;
		gpsUpTimeTick = 0;
		if((gpsdnTimeTick >= GPSDNTIMETICK))//&&(GPS_Valid_flag))
		{
			gpsdnTimeTick = 0;
			GPS_ON_Flag = 0;
			GPS_OFF_Flag = 1;
			Gps_sendok = 0;
			gpsaccuratetick = 0;
   		GPS_SW(0);//关闭串口
			GPS_LP_flag=1;
			printf("<GPSOFF>\r\n");
		}
	}
	if((GPS_OFF_Flag)&&(!GPS_ON_Flag))
	{
		gpsUpTimeTick ++;
		gpsdnTimeTick = 0;
		Gps_sendok = 0;
		if(gpsUpTimeTick >= ((gpsUPTimeLEN-3)*1000/TIMETICKPERIOD))  //提前3秒开GPS
		{
			gpsUpTimeTick = 0;
			GPS_ON_Flag = 1;
			GPS_OFF_Flag = 0;
			gpsaccuratetick = 0;
			GPS_SW(1);
			GPS_LP_flag=0;
			printf("<GPSONN>\r\n");
		}
	}
}

/*******************************************************************************
* Function Name  : Gps_Handler
* Description    : Gps_Handler
* Input          :
* Return         :
*******************************************************************************/
void Gps_Handler(void)
{
	u8	mok_buf[1];
	u8	settime[6];

	if(gpsoutputset>=4)//1s
	{
		gpsoutputset=0;
		Usart_SendString(USART2, "$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28\r\n");
	}

	if(NMEA_GPRMC_Analysis(&gpsx, Rx2Buf)&&NMEA_GPGGA_Analysis(&gpsx, Rx2Buf))
	{
		if((Latitude100000 == GGALatitude100000)&&(Longitude100000 == GGALongitude100000))
		{
			if(GPS_Valid_flag)
			{
				if(!rtc_updata_flag)
				{
					rtc_updata_flag=1;
					settime[0]=gpsx.utc.year;
					settime[1]=gpsx.utc.month;
					settime[2]=gpsx.utc.date;
					settime[3]=gpsx.utc.hour;
					settime[4]=gpsx.utc.min;
					settime[5]=gpsx.utc.sec;
					RTC_OP(settime,RTC_SETTIME_BCD);//定位成功后，更新下时间
				}
				gpsaccuratetick++;
				if(gpsaccuratetick<3)  //过滤GPS开机后前2个有效定位点 ...
				{
					printf("过滤掉第%d个定位点\r\n",gpsaccuratetick);
					return;
				}
			}
			if((TCPIP_been_OK&0X03)==0X03)
			{
				if(gpsUPTimeLEN)
				{
					gprsOFF_GpsSendQuick(); 	//发送盲点
					if((Gps_sendok == 0)&&(GPS_ON_Flag)&&(!GPS_OFF_Flag))
					{
						if(GPS_Valid_flag)
						{
							GpsLocationOperate(&gpsx,mok_buf,DEVICE_LOCATION,TCPLINK);

							printf("<<<<<<GPS_OK>>>>>>\r\n");
							Gps_sendok = 1;
							gpsdnTimeTick = GPSDNTIMETICK;
						}
					}
				}
			}
			else//GPRS盲区定时存储//
			{
				if(gpsUPTimeLEN)
				{
					if(GPS_Valid_flag)		  //存储精确定位的gps...
					{
						if((Gps_sendok == 0)&&(GPS_ON_Flag)&&(!GPS_OFF_Flag))
						{
							gprsOFF_GpsStore(&gpsx);
							gpsdnTimeTick = GPSDNTIMETICK;
							Gps_sendok = 1;
						}
					}
				}
			}
		}
	}
}

/*******************************************************************************
* Function Name  : TemporaryCloseInterrupt
* Description    : 发送盲点的时候关中断
* Input          :
* Return         :
*******************************************************************************/
void TemporaryCloseInterrupt(void)
{
/*
	TIMER_A_stop(TIMER_A0_BASE);  //  低功耗定时器
	TIMER_A_stop(TIMER_A1_BASE);  //  gps 定时器

	USCI_A_UART_clearInterruptFlag(USCI_GPSCOM_BASE,USCI_A_UART_RECEIVE_INTERRUPT);  // GPS 串口
	USCI_A_UART_disableInterrupt(USCI_GPSCOM_BASE,USCI_A_UART_RECEIVE_INTERRUPT);	// 禁止接收和发送中断
*/
	RFINT_off();
}
/*******************************************************************************
* Function Name  : TemporaryOpenInterrupt
* Description    : 盲点发送完时候打开中断
* Input          :
* Return         :
*******************************************************************************/
void TemporaryOpenInterrupt(void)
{
/*
	TIMER_A_startCounter(TIMER_A0_BASE,TIMER_A_UP_MODE);  //  低功耗定时器
	TIMER_A_startCounter(TIMER_A1_BASE,TIMER_A_UP_MODE);   //gps开关定时器

	USCI_A_UART_clearInterruptFlag(USCI_GPSCOM_BASE,USCI_A_UART_RECEIVE_INTERRUPT);
	USCI_A_UART_enableInterrupt(USCI_GPSCOM_BASE,USCI_A_UART_RECEIVE_INTERRUPT);			// 禁止接收和发送中断
*/
	RFINT_en();
}
/*******************************************************************************
* Function Name  : gprsIN_GpsStore
* Description    : 实时存储10个最近的定位点
* Input          :
* Return         :
*******************************************************************************/
void gprsIN_GpsStore(u8 * buf)
{
/*
	u8	i,j;

	for(i = 0;i < InLineGPSnum;i++)
	{
		if( InLineGPSbuf[i][0] != '&')
			break;
	}

	if(i >= InLineGPSnum)
	{
		for(j = 0;j< InLineGPSnum -1;j++)
		{
			memcopy((u8*)&InLineGPSbuf[j][0],(u8*)&InLineGPSbuf[j+1][0],(LineGPSLength+1));
		}
		InLineGPSbuf[InLineGPSnum -1][0]=0;
		i = InLineGPSnum -1;
	}

	memcopy((u8*)&InLineGPSbuf[i][1],buf,LineGPSLength);
	InLineGPSbuf[i][0] ='&';
	UARTWrite((u8*)"InLineGPSbuf\r\n",14,DEBUG_COM);
	SendData2TestCOM((u8 *)& i,1, DEBUG_COM);

	return;
	*/
}
/*******************************************************************************
* Function Name  : gprsOFF_GpsStore
* Description    : 盲点存储在EE
* Input          :
* Return         :
*******************************************************************************/
void gprsOFF_GpsStore(nmea_msg *gps)
{
	u8 index;
	u8 buff[16];

	EEPROM_OP((u8*)&buff[0],OFFLINEGPSBASE,1,MODE_R);
	EEPROM_OP((u8*)&OffLineGPSindex,OFFLINEGPSINDEX_ADDR,2,MODE_R);

	if(buff[0] != '&')
		OffLineGPSindex = 0;
	else
		OffLineGPSindex = (OffLineGPSindex +1)%((u16)(OffLineGPSnum));

	EEPROM_OP((u8*)&OffLineGPSindex,OFFLINEGPSINDEX_ADDR,2,MODE_W);

	index = 0;
	buff[index] = '&';
	index++;
	buff[index] = ((TT_Alarm_Status&ayjks)?(1):(0))*8+((TT_Alarm_Status& aLockCUT)?(1):(0))*4+((TT_Alarm_Status& aLockOPEN)?(1):(0))*2+ ((TT_Alarm_Status& aVDDLOW)?(1):(0));
	//======== 纬度=============//
	buff[index] = (u8)((((gps->latitude)*10)&0xff000000)>>24);
	index++;
	buff[index] = (u8)((((gps->latitude)*10)&0xff0000)>>16);
	index++;
	buff[index] = (u8)((((gps->latitude)*10)&0xff00)>>8);
	index++;
	buff[index] = (u8)(((gps->latitude)*10)&0xff);
	index++;
	//////////////////////////////////

	//======== 经度=============//
	buff[index] = (u8)(((gps->longitude*10)&0xff000000)>>24);             //
	index++;
	buff[index] = (u8)(((gps->longitude*10)&0xff0000)>>16);
	index++;
	buff[index] = (u8)(((gps->longitude*10)&0xff00)>>8);
	index++;
	buff[index] = (u8)((gps->longitude*10)&0xff);
	index++;
	//======== UTC=============//
	buff[index] = gps->utc.year;
	index++;
	buff[index] = gps->utc.month;
	index++;
	buff[index] = gps->utc.date;
	index++;
	buff[index] = gps->utc.hour;
	index++;
	buff[index] = gps->utc.min;
	index++;
	buff[index] = gps->utc.sec;
	index++;
	EEPROM_OP(buff,((u32)(OFFLINEGPSBASE) + (u32)(OffLineGPSindex)*(LineGPSLength)),LineGPSLength,MODE_W);

	printf("盲点存储 %d\r\n",OffLineGPSindex);
}
/*******************************************************************************
* Function Name  : gprsOFF_GpsSendQuick
* Description    : 发送盲点
* Input          :
* Return         :
*******************************************************************************/
void gprsOFF_GpsSendQuick(void)
{
	u8 temp[300],buffer[28];
	u16 gpsnum,index;
	u8 offlinegps,bindex;
	u8 offgpsdata[16];

	EEPROM_OP((u8*)&OffLineGPSindex,OFFLINEGPSINDEX_ADDR,2,MODE_R);
	EEPROM_OP((u8*)&offlinegps,((u32)(OFFLINEGPSBASE) + (u32)(OffLineGPSindex)*(LineGPSLength)),1,MODE_R);

	if(offlinegps!= '&' )
	{
		return;
	}
	if(AuthorizeProtocol.authresult != 'Y')
	{
		return;
	}
	TemporaryCloseInterrupt();

	EEPROM_OP((u8*)&offlinegps,((u32)(OFFLINEGPSBASE) + ((u32)(OffLineGPSnum) - 1)*(LineGPSLength)) ,1,MODE_R);
	if(offlinegps == '&')   //数据存满...
	{
		gpsnum=0;
		index = OffLineGPSindex;
		while(1)
		{
			FEED_WTDG;
			index = (index +1)%((u16)(OffLineGPSnum));
			EEPROM_OP(offgpsdata,((u32)(OFFLINEGPSBASE) + (u32)(index)*(LineGPSLength)) ,LineGPSLength,MODE_R);
			bindex = 0;
			if(offgpsdata[0] == '&')  //offgpsdata[0]
			{
				 //报警标志           //offgpsdata[1]
				buffer[bindex] = 0x00;
				bindex++;
				buffer[bindex] = 0x00;
				if(offgpsdata[1]&0x08)
					buffer[bindex] |=0x02;
				if(offgpsdata[1]& 0x04)
					buffer[bindex] |=0x01;
				bindex++;
				buffer[bindex] = 0x00;
				if(offgpsdata[1]& 0x02)
					buffer[bindex] |=0x80;
				bindex++;
				buffer[bindex] = 0;
				if(offgpsdata[1]&0x01)
					buffer[bindex] |=0x80;
				bindex++;

				//===  状态位===========//

				buffer[bindex] = 0x00;
				bindex++;
				buffer[bindex] = 0x00;
				bindex++;
				buffer[bindex] = 0x00;
				bindex++;
				buffer[bindex] = 0x00;
				bindex++;
				memcopy(buffer+bindex,offgpsdata+2,8);  //offgpsdata[2]--[9]   纬度经度
				bindex +=8;
				memREset(buffer+bindex,0x00,6);         //高度速度方向
				bindex +=6;
				memcopy(buffer+bindex,offgpsdata+10,6);  //offgpsdata[10]--[15]  UTC
				bindex +=6;

				memcopy(temp+gpsnum*28,buffer,28);

				offgpsdata[0]= 0;
				EEPROM_OP((u8*)&offgpsdata[0],((u32)(OFFLINEGPSBASE) + (u32)index*(LineGPSLength)) ,1,MODE_W);
				gpsnum++;

				if(gpsnum >= 10)
				{
					NewPacket_KY(temp,gpsnum*28,DEVICE_OFFLINELOCATION,TCPLINK);
					gpsnum= 0;
					SysTick_Delay_Ms(5);
				}
			}
			else
			{
				if(gpsnum > 0)
				{

					NewPacket_KY(temp,gpsnum*28,DEVICE_OFFLINELOCATION,TCPLINK);
					SysTick_Delay_Ms(5);
				}
				UARTWrite((u8*)"GPS QKsendOK1\r\n",15,DEBUG_COM);
				OffLineGPSindex = 0;
				EEPROM_OP((u8*)&OffLineGPSindex,OFFLINEGPSINDEX_ADDR,2,MODE_W);
				break;
			}
		}
	}
	else
	{
		gpsnum=0;
		index = 0;
		while(1)
		{
			FEED_WTDG;
			EEPROM_OP(offgpsdata,((u32)(OFFLINEGPSBASE) + (u32)(index)*(LineGPSLength)) ,LineGPSLength,MODE_R);
			bindex = 0;

			if(offgpsdata[0] == '&')  //offgpsdata[0]
			{
				 //报警标志           //offgpsdata[1]
				buffer[bindex] = 0x00;
				bindex++;
				buffer[bindex] = 0x00;
				if(offgpsdata[1]&0x08)
					buffer[bindex] |=0x02;
				if(offgpsdata[1]& 0x04)
					buffer[bindex] |=0x01;
				bindex++;
				buffer[bindex] = 0x00;
				if(offgpsdata[1]& 0x02)
					buffer[bindex] |=0x80;
				bindex++;
				buffer[bindex] = 0;
				if(offgpsdata[1]&0x01)
					buffer[bindex] |=0x80;
				bindex++;

				//===  状态位===========//

				buffer[bindex] = 0x00;
				bindex++;
				buffer[bindex] = 0x00;
				bindex++;
				buffer[bindex] = 0x00;
				bindex++;
				buffer[bindex] = 0x00;
				bindex++;
				memcopy(buffer+bindex,offgpsdata+2,8);  //offgpsdata[2]--[9]   纬度经度
				bindex +=8;
				memREset(buffer+bindex,0x00,6);         //高度速度方向
				bindex +=6;
				memcopy(buffer+bindex,offgpsdata+10,6);  //offgpsdata[10]--[15]  UTC
				bindex +=6;

				memcopy(temp+gpsnum*28,buffer,28);

				offgpsdata[0]= 0;
				EEPROM_OP((u8*)&offgpsdata[0],((u32)(OFFLINEGPSBASE) + (u32)index*(LineGPSLength)) ,1,MODE_W);
				gpsnum++;
				index++;
				if(gpsnum >= 10)
				{
					NewPacket_KY(temp,gpsnum*28,DEVICE_OFFLINELOCATION,TCPLINK);
					gpsnum= 0;
					SysTick_Delay_Ms(5);
				}
			}
			else
			{
				if(gpsnum > 0)
				{
					NewPacket_KY(temp,gpsnum*28,DEVICE_OFFLINELOCATION,TCPLINK);
					SysTick_Delay_Ms(5);
				}
				UARTWrite((u8*)"GPS QKsendOK2\r\n",15,DEBUG_COM);
				OffLineGPSindex = 0;
				EEPROM_OP((u8*)&OffLineGPSindex,OFFLINEGPSINDEX_ADDR,2,MODE_W);
				break;
			}
		}
	}
	TemporaryOpenInterrupt();
}

/*******************************************************************************
* Function Name  : GpsLocationOperate
* Description    : 0200
* Input          :
* Return         :
*******************************************************************************/
void GpsLocationOperate(nmea_msg *gps,u8* Serverindex,u16 mode,u8 link)
{
	u8 tmp[30],index;

	if(mode == DEVICE_LOCATION)
	{
		index = 0;
	}
	else if(mode == CHECK_LOCATION_ANSWER)
	{
		tmp[index] = Serverindex[0];index++;
		tmp[index] = Serverindex[1];index++;
	}

	 //报警标志 0000 0000   0000 0011   1000 0000   1000 0000
	tmp[index] = 0x00;index++;
	
	tmp[index] = 0x00;
	if(TT_Alarm_Status& ayjks)				//非法开锁
		tmp[index] |=0x02;
	
	if(TT_Alarm_Status& aLockCUT)			//拆壳
		tmp[index] |=0x01;
	index++;
	
	tmp[index] = 0x00;
//	if(TT_Alarm_Status& aLockOPEN)		//锁杆剪短
//		tmp[index] |=0x80;
	index++;
	
	tmp[index] = 0x00;
	if(TT_Alarm_Status& aVDDLOW)			//低电压
		tmp[index] |=0x80;
	index++;

	//===  状态位===========//

	tmp[index] = 0x00;
	index++;
	tmp[index] = 0x00;
	index++;
	tmp[index] = 0x00;
	index++;
	tmp[index] = 0x00;
	index++;

	//======== 纬度=============//
	tmp[index] = (u8)((((gps->latitude)*10)&0xff000000)>>24);
	index++;
	tmp[index] = (u8)((((gps->latitude)*10)&0xff0000)>>16);
	index++;
	tmp[index] = (u8)((((gps->latitude)*10)&0xff00)>>8);
	index++;
	tmp[index] = (u8)(((gps->latitude)*10)&0xff);
	index++;
	//////////////////////////////////

	//======== 经度=============//
	tmp[index] = (u8)(((gps->longitude*10)&0xff000000)>>24);             //
	index++;
	tmp[index] = (u8)(((gps->longitude*10)&0xff0000)>>16);
	index++;
	tmp[index] = (u8)(((gps->longitude*10)&0xff00)>>8);
	index++;
	tmp[index] = (u8)((gps->longitude*10)&0xff);
	index++;
	//////////////////////////////////

	//======== 高程=============//
	tmp[index] = (u8)(((gps->altitude)&0xff00)>>8);
	index++;
	tmp[index] = (u8)((gps->altitude)&0xff);
	index++;
	//////////////////////////////////

	//======== 速度=============//
	tmp[index] = (u8)(((gps->speed)&0xff00)>>8);
	index++;
	tmp[index] = (u8)((gps->speed)&0xff);
	index++;
	//////////////////////////////////

	//========方向=============//
	tmp[index] = (u8)(((gps->Course)&0xff00)>>8);
	index++;
	tmp[index] = (u8)((gps->Course)&0xff);
	index++;
	//////////////////////////////////

	//======== UTC=============//
	tmp[index] = gps->utc.year;
	index++;
	tmp[index] = gps->utc.month;
	index++;
	tmp[index] = gps->utc.date;
	index++;
	tmp[index] = gps->utc.hour;
	index++;
	tmp[index] = gps->utc.min;
	index++;
	tmp[index] = gps->utc.sec;
	index++;
	//////////////////////////////////
	NewPacket_KY(tmp,index,mode,link);


}


