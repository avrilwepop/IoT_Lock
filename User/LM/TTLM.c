#include "gbprotocol.h"
#include "global.h"
#include "TTLM.H"
#include "rfid.h"
#include "gps.h"
#include "config.h"
#include "crcencoder.h"
#include "gprs.h"
#include "BSprotocol.h"
#include "common.h"
#include "lp_mode.h"
#include "main.h"

//========================================================================//
const u8 WNMM[]={0x27,0x0F,0x00,0x02,0x3F,0x3A,0X11,0X22,0X33,0X55};////9999147258//

u32 LockTicker = 0;
u8 AD0PS_cnt=0;
u8 TT_Alarm_Status=0;
u8 ACC_Off_flag = '0';
u8 CanPWupModule_Cnt = 0;

u8 NeedSendAlarmCount = 0;
u16 VinsCount = 240;
u8 Readyrstcnt = 0;
u8 NetModuleSTATUS = 0;
u8 NeedSendAlarm=0;
u8 Tcpsendflag = 0;
u8 tcpsendtick = 0;
u8 qisacktick = 0;
u8 HallcheckBtick =0;
u8 HallcheckCtick =0;
u8 checkpowofftick =0;

unsigned int StartTime;//
unsigned int NewStartTime;
u8 GB_LOCKIDbuf[9];
u8 GBLOCKIDASC[14];
u8 LockStatus;//EE//
u8 MoToStatus;//0:open;2:close//
u8 AlarmType;
u8 RFTxBuf[100];//
u8 AlarmCnt;
u8 AlarmTimebuf[8];
u32 SeqCN;
u8 TotalRecNum1;//iee
u16 RecordIndex;
u8 SecKeyBuf[11];
u8 SecKeyBuf6[6];

u16 RecReTrTimer;
u16 NewReTrTimer;
u16 SysTimer;
u8 Vdd;
u8 GBVdd;
u8 CertificateNo[18];   //单证号...
u8 StartAddress[4];		//起运地...
u8 EndAddress[4];		//目的地...
u8 NeedSaveFlag;
u8 Needresetflag[3];
u16 Adc_Vdd;
u8 Send2CCBuf[100];
u8 CheckLowPower;
u8 CheckLowStep;
u16 Lowpowertick;

u8 TCPIP_been_OK;
u8 GPRSReConnectCnt;
u8 ModuleNeedCutRST;
u8 ModuleTempClosedCnt;
u8 HadCut_MPW_BOLV;
u8 GPS_ON_Flag;
u8 GPS_OFF_Flag;
u8 SMS_in_Num[20];
u8 Tick_250ms;
_s_Time rtc_time;

u8 GPRS_LP_flag=0;//20180201添加，各部分休眠标志，0不休眠，1进入休眠

#if NEED_GPS
u8 GPS_LP_flag=0;	//20180201添加，各部分休眠标志，0不休眠，1进入休眠
#else
u8 GPS_LP_flag=1;	//20180201添加，各部分休眠标志，0不休眠，1进入休眠
#endif

u8 KEY_LP_flag=0;	//20180201添加，各部分休眠标志，0进入，>0不入休眠
u8 RF_LP_flag=1;	//20180201添加，各部分休眠标志，0不休眠，1进入休眠

u8 vddlow_flag=0;

/*******************************************************************************
* Function Name  : Lock_EProm_Reset
* Description    : 整个参数恢复出厂值
* Input          : None.
* Return         : None.
*******************************************************************************/
void Lock_EProm_Reset(u8 mode)
{
	u8 tmp[18];
	u8 i;

	IPset(0x30,tmp,0);

	//清所有报警//
	TT_Alarm_Status=0;
	EEPROM_OP((u8*)&TT_Alarm_Status,TT_Alarm_Status_ADDR,1,MODE_W);

	gpsUPTimeLEN= 30; //30S
	EEPROM_OP((u8*)&gpsUPTimeLEN,gpsUPTimeLEN_ADDR,4,MODE_W);  
	gpsUPTimeLENsleep = 3600;
	EEPROM_OP((u8*)&gpsUPTimeLENsleep,gpsUPTimeLENsleep_ADDR,4,MODE_W);
	
	tmp[0]=0X00;
	tmp[1]=0x00;
	tmp[2]=0x00;
	tmp[3]=0X00;
	tmp[4]=0x00;
	tmp[5]=0x00;
	tmp[6]=0x00;
	tmp[7]=0x00;
	EEPROM_OP((u8*)&tmp[0],AlarmTime_ADDR,8,MODE_W);
	EEPROM_OP((u8*)&tmp[0],LockStatus_ADDR,1,MODE_W);
	EEPROM_OP((u8*)&tmp[0],AlarmType_ADDR,1,MODE_W);
	EEPROM_OP((u8*)&tmp[0],MoToStatus_ADDR,1,MODE_W);
	tmp[0] = 0x31;
	tmp[1] = 0x32;
	tmp[2] = 0x33;
	tmp[3] = 0x34;
	tmp[4] = 0x35;
	tmp[5] = 0x36;
	tmp[6] = 0x37;
	tmp[7] = 0x38;
	tmp[8] = 0x39;
	tmp[9] = 0x30;
	EEPROM_OP((u8*)&tmp[0],SecKey_ADDR,10,MODE_W);
	EEPROM_OP((u8*)&tmp[0],SecKey_ADDR2,10,MODE_W);
	tmp[0] = 0x00;
	tmp[1] = 0x00;
	tmp[2] = 0x00;
	tmp[3] = 0x00;
	EEPROM_OP((u8*)&tmp[0],TotalRecNum_ADDR,1,MODE_W);
	EEPROM_OP((u8*)&tmp[0],RECORDINDEX_ADDR,2,MODE_W);
	EEPROM_OP((u8*)&tmp[0],SeqCN_ADDR,4,MODE_W);
	for(i=0;i<MAXLockRECVOL;i++)
	{
		EEPROM_OP((u8 *)&tmp[0], TAGRECORDBUFF+i*HVOL, 2, MODE_W);
	}

////////////////////////////////////
//		tmp[0] = 0x01;	tmp[1] = 0x90;
//		tmp[2] = 0x00;	tmp[3] = 0x00;
//		tmp[4] = 0x00;	tmp[5] = 0x01;
//		EEPROM_OP((u8*)&tmp[0],DEV_SIMID_ADD,6,MODE_W);
	///////////////////////////////////

	if(mode == 1)
	{

		UARTWrite((u8*)"@answ@Restore factory settings@end@",35,0);
		SysTick_Delay_Ms(20);
		while(1);
	}
}
/*******************************************************************************
* Function Name  : LockTansGBLOCKID
* Description    : 锁号转换成ascii
* Input          : None.
* Return         : None.
*******************************************************************************/
void LockTansGBLOCKID(void)
{
	u32 Di;

	GBLOCKIDASC[0] = GB_LOCKIDbuf[0];
	GBLOCKIDASC[1] = GB_LOCKIDbuf[1];
	GBLOCKIDASC[2] = GB_LOCKIDbuf[2];
	GBLOCKIDASC[3] = GB_LOCKIDbuf[3];
	Di = (((u32)GB_LOCKIDbuf[4])<<24)+(((u32)GB_LOCKIDbuf[5])<<16)+(((u32)GB_LOCKIDbuf[6])<<8)+(((u32)GB_LOCKIDbuf[7]));

	GBLOCKIDASC[4] = Di/(u32)1000000000+0x30;
	GBLOCKIDASC[5] = (Di%(u32)1000000000)/(u32)100000000+0x30;
	GBLOCKIDASC[6] = (Di%(u32)100000000)/(u32)10000000+0x30;
	GBLOCKIDASC[7] = (Di%(u32)10000000)/(u32)1000000+0x30;
	GBLOCKIDASC[8] = (Di%(u32)1000000)/(u32)100000+0x30;
	GBLOCKIDASC[9] = (Di%(u32)100000)/(u32)10000+0x30;
	GBLOCKIDASC[10] = (Di%(u32)10000)/(u32)1000+0x30;
	GBLOCKIDASC[11] = (Di%(u32)1000)/(u32)100+0x30;
	GBLOCKIDASC[12] = (Di%(u32)100)/(u32)10+0x30;
	GBLOCKIDASC[13] = (Di%(u32)10)+0x30;
}

/*******************************************************************************
* Function Name  : Lock_Para_Init
* Description    : 锁参数初始化
* Input          : None.
* Return         : None.
*******************************************************************************/
void Lock_Para_Init(void)
{
	printf("U-Lock VVV\r\n");
	Lock_Para_Get();
	RecReTrTimer = WORDMS;
	NewReTrTimer = WORDMS;
	Send2CCBuf[0]=0;
	printf("20180306\r\n");
}
/*******************************************************************************
* Function Name  : E2_LockParaGet
* Description    : 开机之后  获取锁的参数
* Input          : None.
* Return         : None.
*******************************************************************************/
void Lock_Para_Get(void)
{
	u8 i;
	ClrLockOPRec(cGPRS_TRLIST);
	//锁ID号
	EEPROM_OP((u8*)GB_LOCKIDbuf,GB_LOCKID_ADDR,8,MODE_R);
	if((GB_LOCKIDbuf[0]!='C')||(GB_LOCKIDbuf[1]!='N'))
	{
		GB_LOCKIDbuf[0]='C';
		GB_LOCKIDbuf[1]='N';
		GB_LOCKIDbuf[2]='S';
		GB_LOCKIDbuf[3]='M';

		GB_LOCKIDbuf[4]=0xA7;
		GB_LOCKIDbuf[5]=0xD8;
		GB_LOCKIDbuf[6]=0xF2;
		GB_LOCKIDbuf[7]=0x11;
	}
	LockTansGBLOCKID();
	printf("Lock ID:");
	UARTWrite(GBLOCKIDASC,14,0);
	UARTWrite((u8 *)"\r\n",2,0);

	//报警时间
	EEPROM_OP(AlarmTimebuf,AlarmTime_ADDR,8,MODE_R);
	if((AlarmTimebuf[0]==0xff)&&(AlarmTimebuf[1]==0xff)&&(AlarmTimebuf[2]==0xff)
		&&(AlarmTimebuf[3]==0xff))
	{
		for(i = 0;i< 8;i++)
			AlarmTimebuf[i] = 0;
	}

	//锁状态
	EEPROM_OP((u8*)&LockStatus,LockStatus_ADDR,1,MODE_R);

	//报警类型
	EEPROM_OP((u8*)&AlarmType,AlarmType_ADDR,1,MODE_R);
	if(AlarmType == 0xff)
		AlarmType = 0;

	//电机状态
	EEPROM_OP((u8*)&MoToStatus,MoToStatus_ADDR,1,MODE_R);

	//秘钥
	EEPROM_OP(SecKeyBuf,SecKey_ADDR,10,MODE_R);
	if((SecKeyBuf[1] == 0xff&&SecKeyBuf[3] == 0xff&&SecKeyBuf[5] == 0xff&&SecKeyBuf[7] == 0xff&&SecKeyBuf[9] == 0xff)
	    || (SecKeyBuf[1] == 0x00&&SecKeyBuf[3] == 0x00&&SecKeyBuf[5] == 0x00&&SecKeyBuf[7] == 0x00&&SecKeyBuf[9] == 0x00))
	{
		SecKeyBuf[0] = 0x31;
		SecKeyBuf[1] = 0x32;
		SecKeyBuf[2] = 0x33;
		SecKeyBuf[3] = 0x34;
		SecKeyBuf[4] = 0x35;
		SecKeyBuf[5] = 0x36;
		SecKeyBuf[6] = 0x37;
		SecKeyBuf[7] = 0x38;
		SecKeyBuf[8] = 0x39;
		SecKeyBuf[9] = 0x30;
		EEPROM_OP(SecKeyBuf,SecKey_ADDR,10,MODE_W);
	}

	//记录总数
	EEPROM_OP((u8*)&TotalRecNum1,TotalRecNum_ADDR,1,MODE_R);
	if(TotalRecNum1>MAXBC_NUM)
	{
		TotalRecNum1 = MAXBC_NUM;
	}

	EEPROM_OP((u8*)&RecordIndex,RECORDINDEX_ADDR,2,MODE_R);

	EEPROM_OP((u8*)&SeqCN,SeqCN_ADDR,4,MODE_R);

	switch(LockStatus)
	{
		case LOCKWAIT:
		case UNSEAL:
		case SEAL:
		case ALARM:
		case EXALARM:
			break;

		default:
			LockStatus=LOCKWAIT;
			break;
	}

//	UARTWrite((u8*)"锁号\r\n",6,0);//DEBUGG
//	SendData2TestCOM((u8*)&GB_LOCKIDbuf,8,0);//DEBUGG
//	UARTWrite((u8*)"报警时间\r\n",10,0);//DEBUGG
//	SendData2TestCOM((u8*)&AlarmTimebuf,8,0);//DEBUGG
//	UARTWrite((u8*)"锁状态\r\n",8,0);//DEBUGG
//	printf("<%d>\r\n",LockStatus);
//	UARTWrite((u8*)"报警类型\r\n",10,0);//DEBUGG
//	printf("<%d>\r\n",AlarmType);
//	UARTWrite((u8*)"电机状态\r\n",10,0);//DEBUGG
//	printf("<%d>\r\n",MoToStatus);
//	UARTWrite((u8*)"密钥\r\n",6,0);//DEBUGG
//	SendData2TestCOM((u8*)&SecKeyBuf,10,0);//DEBUGG
//	UARTWrite((u8*)"记录总数\r\n",14,0);//DEBUGG
//	printf("<%d>\r\n",TotalRecNum1);
//	UARTWrite((u8*)"RecordIndex\r\n",13,0);//DEBUGG
//	printf("<%d>\r\n",RecordIndex);
//	UARTWrite((u8*)"单证号\r\n",8,0);//DEBUGG
//	SendData2TestCOM(CertificateNo,18,0);//DEBUGG
//	UARTWrite((u8*)"起运地\r\n",8,0);//DEBUGG
//	SendData2TestCOM(StartAddress,4,0);//DEBUGG
//	UARTWrite((u8*)"目的地\r\n",8,0);//DEBUGG
//	SendData2TestCOM(EndAddress,4,0);//DEBUGG
}

/*******************************************************************************
* Function Name  : LockTimHandler
* Description    : GPRS有的指令需要重发
* Input          : None.
* Return         : None.
*******************************************************************************/
void LockTimHandler(void)
{
	StartTime=RecReTrTimer;
	if(GetTimer>(DevParameters.GprsRetryTimeout*4))      //   250ms*4
	{
		SysTick_Delay_Ms(5);
		RecReTrTimer=WORDMS;
		if((TCPIP_been_OK&0X03)==0X03)
		{
			if(AuthorizeProtocol.authresult == 'Y')
				LockRecReUP();
		}
	}
	NewStartTime = NewReTrTimer;
	if(NewGetTimer > (DevParameters.GprsRetryTimeout*4))
	{
		NewReTrTimer = WORDMS;
		if((TCPIP_been_OK&0X03)==0X03)
		{
			NewLockRecReUP();
		}
	}
}


/*******************************************************************************
* Function Name  : GBRec_FIFO
* Description    : 老的协议  存储记录
* Input          : None.
* Return         : None.
*******************************************************************************/
u8 GBRec_FIFO(u8 *recbuf,u8 newbc,u8 mode)
{
/*
	u8 tmpbuf[50];
    //u8 i;
	//01是最近时间的操作记录，03是最早时间的操作记录

	if(mode == MODE_R)
	{
		if((newbc>=1)&&(newbc<=TotalRecNum1))
		{
			if(TotalRecNum1 < MAXBC_NUM)
				EEPROM_OP(tmpbuf,((u32)(LockEVENT1_ADDR) +(u32)(newbc-1)*RECORDDATALEN),RECORDDATALEN,MODE_R);
			else
				EEPROM_OP(tmpbuf,((u32)(LockEVENT1_ADDR)+((u32)(RecordIndex + newbc -1)% MAXBC_NUM)*RECORDDATALEN),RECORDDATALEN,MODE_R);
		}

		return (RECORDDATALEN);
	}
	else if((mode == MODE_W)||(mode == MODE_ALARM))
	{
		//序号	  事件代码	单证号	操作时间	起运地	目的地	阅读器ID
		//1字节 	1字节		18字节	 8字节		 4字节	4字节			   8字节
		tmpbuf[0]=1;
		tmpbuf[1]=recbuf[0];
		memcopy(tmpbuf+2,CertificateNo,18);//单证号//
		//RTC_OP(tmpbuf+20,GET_RTC_DTIME_GB); //操作时间
		memcopy(tmpbuf+28,(u8*)StartAddress,4);//起运地
		memcopy(tmpbuf+32,(u8*)EndAddress,4); //目的地
		if(mode==MODE_W)
		{
			memcopy(tmpbuf+36,(u8*)RFRxBuf+10,8);//READER_ID//
		}
		else
		{
			memREset(tmpbuf+36,0x33,8);
		}
		EEPROM_OP(tmpbuf,((u32)(LockEVENT1_ADDR)+((u32)(RecordIndex)*RECORDDATALEN)),RECORDDATALEN,MODE_W);
		RecordIndex++;
		RecordIndex = RecordIndex%MAXBC_NUM;
		SendData2TestCOM((u8 *)&RecordIndex,2,0);//DEBUGG
		EEPROM_OP((u8*)&RecordIndex,RECORDINDEX_ADDR,2,MODE_W);
		if(TotalRecNum1<MAXBC_NUM)
		{
			TotalRecNum1++;
			EEPROM_OP((u8*)&TotalRecNum1,TotalRecNum_ADDR,1,MODE_W);
		}
		else
		{
			TotalRecNum1=MAXBC_NUM;
		}

		return (RECORDDATALEN);

	}
	else
	*/
		return (0);
}
/*******************************************************************************
* Function Name  : GBRec_Center_FIFO
* Description    : 新的协议 存储记录
* Input          : None.
* Return         : None.
*******************************************************************************/
u8 GBRec_Center_FIFO(u8 *recbuf,u8 newbc,u8 mode)
{
	u8 tmpbuf[50];
	//u8 i;

	if(mode == MODE_R)
	{
		if((newbc>=1)&&(newbc<=TotalRecNum1))
		{
			if(TotalRecNum1 < MAXBC_NUM)
				EEPROM_OP(tmpbuf,((u32)(LockNEWEVENT_ADDR)+(u32)(newbc-1)*NEWRECORDDATALEN),NEWRECORDDATALEN,MODE_R);
			else
				EEPROM_OP(tmpbuf,((u32)(LockNEWEVENT_ADDR)+((u32)(RecordIndex + newbc -1)% MAXBC_NUM)*NEWRECORDDATALEN),NEWRECORDDATALEN,MODE_R);
		}
		return (NEWRECORDDATALEN);
	}
	else if((mode == MODE_W)||(mode == MODE_ALARM))
	{

		//序号	  	操作时间	     操作类型
		//1字节 		 6字节		 			   1字节
		tmpbuf[0]=1;

		RTC_OP( tmpbuf+1,RTC_GETTIME_BCD);

		tmpbuf[7]=recbuf[0];

		EEPROM_OP(tmpbuf,((u32)(LockNEWEVENT_ADDR)+((u32)(RecordIndex)*NEWRECORDDATALEN)),NEWRECORDDATALEN,MODE_W);
		RecordIndex++;
		RecordIndex = RecordIndex%MAXBC_NUM;
		SendData2TestCOM((u8 *)&RecordIndex,2,0);//DEBUGG
		EEPROM_OP((u8*)&RecordIndex,RECORDINDEX_ADDR,2,MODE_W);
		if(TotalRecNum1<MAXBC_NUM)
		{
			TotalRecNum1++;
			EEPROM_OP((u8*)&TotalRecNum1,TotalRecNum_ADDR,1,MODE_W);
		}
		else
		{
			TotalRecNum1=MAXBC_NUM;
		}

		return (NEWRECORDDATALEN);
	}
	else
		return (0);
}
/*******************************************************************************
* Function Name  : MotoDriver
* Description    : 驱动电机
* Input          : None.
* Return         : None.
*******************************************************************************/
u8 MotoDriver(u8 type)
{
	if(type==bLOCK)
	{
		MOTOR_CLOSE;
		MoToStatus=2;
	}
	else
	{
		MOTOR_OPEN;
		MoToStatus=0;
	}
	FEED_WTDG;
	SysTick_Delay_Ms(1000);
	FEED_WTDG;

	MOTOR_STOP;
	EEPROM_OP((u8*)&MoToStatus,MoToStatus_ADDR,1,MODE_W);
	return 1;
}

/*******************************************************************************
* Function Name  : ReUnLock
* Description    : 再次解封
* Input          : None.
* Return         : None.
*******************************************************************************/
u8 ReUnLock(void)
{
	MOTOR_OPEN;
	MoToStatus=0;
	FEED_WTDG;
	SysTick_Delay_Ms(500);
	FEED_WTDG;
	MOTOR_STOP;
	EEPROM_OP((u8*)&MoToStatus,MoToStatus_ADDR,1,MODE_W);
	return 1;
}
/*******************************************************************************
* Function Name  : ShowDateTime
* Description    : 时间转换成字符串
* Input          : None.
* Return         : None.
*******************************************************************************/
void ShowDateTime(u8* stime,u8* dtime)
{
	u8 i;
	//2015 12 15 10 17 21
	for(i = 0; i< 4; i++)
		stime[i] = dtime[i];
	stime[4] = '/';
	stime[5] = dtime[4];
	stime[6] = dtime[5];
	stime[7] = '/';
	stime[8] = dtime[6];
	stime[9] = dtime[7];
	stime[10] = '/';
	stime[11] = dtime[8];
	stime[12] = dtime[9];
	stime[13] = ':';
	stime[14] = dtime[10];
	stime[15] = dtime[11];
	stime[16] = ':';
	stime[17] = dtime[12];
	stime[18] = dtime[13];
}
/*******************************************************************************
* Function Name  : LockOPEN_OP
* Description    : 剪断报警
* Input          : None.
* Return         : None.
*******************************************************************************/
void LockOPEN_OP(void)
{
	u8  err=0;

	switch(LockStatus)
	{
		case SEAL:
			LockStatus=ALARM;
			AlarmType&=~OPENALARM;
			AlarmType|=OPENALARM;
			EEPROM_OP((u8*)&LockStatus,LockStatus_ADDR,1,MODE_W);
			EEPROM_OP((u8*)&AlarmType,AlarmType_ADDR,1,MODE_W);
			err=3;
			AlarmCnt=1;
			break;

		case ALARM:
			if((AlarmType&OPENALARM)!=OPENALARM)
			{
				AlarmType&=~OPENALARM;
				AlarmType|=OPENALARM;
				EEPROM_OP((u8*)&AlarmType,AlarmType_ADDR,1,MODE_W);
				err=2;
				AlarmCnt=2;
			}
			break;
		case EXALARM:
			if((AlarmType&OPENALARM)!=OPENALARM)
			{
				AlarmType&=~OPENALARM;
				AlarmType|=OPENALARM;
				EEPROM_OP((u8*)&AlarmType,AlarmType_ADDR,1,MODE_W);
				err=1;
				AlarmCnt++;
			}
			break;
		default:
			break;
	}
	if(err)
	{
		TT_Alarm_Status&=~aLockOPEN;
		TT_Alarm_Status|=aLockOPEN;
		RTC_OP(AlarmTimebuf,RTC_GETTIME_GB);
		EEPROM_OP(AlarmTimebuf,AlarmTime_ADDR,8,MODE_W);
		RFTxBuf[0]=NEW_OPENALARM_EVENT;
		GBRec_Center_FIFO(RFTxBuf,0,MODE_ALARM);
		GpsLocationOperate(&gpsx,RFTxBuf,DEVICE_LOCATION,TCPLINK);
	}
}

/*******************************************************************************
* Function Name  : LockCHAIKE_OP
* Description    : 处理拆壳报警
* Input          : None.
* Return         : None.
*******************************************************************************/
void LockCHAIKE_OP(void)
{
	u8  err=0;

	  switch(LockStatus)
	  {
		  case SEAL:
				LockStatus=ALARM;
			  AlarmType&=~CKALARM;
			  AlarmType|=CKALARM;
			  EEPROM_OP((u8*)&LockStatus,LockStatus_ADDR,1,MODE_W);
			  EEPROM_OP((u8*)&AlarmType,AlarmType_ADDR,1,MODE_W);
				err=3;AlarmCnt=1;
				UARTWrite((u8*)"CHAIKE1\r\n",9,DEBUG_COM);//DEBUGGGGGGGGGGG
		  break;

		  case EXALARM:
			case ALARM:
		    if((AlarmType&CKALARM)!= CKALARM)
				{
					LockStatus=ALARM;
					AlarmType&=~CKALARM;
			    AlarmType|=CKALARM;
			    EEPROM_OP((u8*)&LockStatus,LockStatus_ADDR,1,MODE_W);
			    EEPROM_OP((u8*)&AlarmType,AlarmType_ADDR,1,MODE_W);
          err=2;
          UARTWrite((u8*)"CHAIKE2\r\n",9,DEBUG_COM);//DEBUGGGGGGGGGGG
          AlarmCnt++;
			  }
				break;
			default:
		  break;
	  }
	  if(err)
	  {
			TT_Alarm_Status&=~aLockCUT;
			TT_Alarm_Status|=aLockCUT;
			RTC_OP(AlarmTimebuf,RTC_GETTIME_GB);
 			EEPROM_OP(AlarmTimebuf,AlarmTime_ADDR,8,MODE_W);
		  RFTxBuf[0] = NEW_CKALARM_EVENT;
	    GBRec_Center_FIFO(RFTxBuf,0,MODE_ALARM);
			GpsLocationOperate(&gpsx,RFTxBuf,DEVICE_LOCATION,TCPLINK);
	  }
}
/*******************************************************************************
* Function Name  : LockYJKS_OP
* Description    : 非法开锁
* Input          : None.
* Return         : None.
*******************************************************************************/
void LockYJKS_OP(void)
{
	u8  err=0;
	  switch(LockStatus)
	  {
		  case SEAL:
				LockStatus=ALARM;
			  AlarmType&=~YJKSALARM;
			  AlarmType|=YJKSALARM;
			  EEPROM_OP((u8*)&LockStatus,LockStatus_ADDR,1,MODE_W);
			  EEPROM_OP((u8*)&AlarmType,AlarmType_ADDR,1,MODE_W);
				err=3;AlarmCnt=1;
				UARTWrite((u8*)"YJKS1\r\n",7,DEBUG_COM);//DEBUGGGGGGGGGGG
		  break;

		  case EXALARM:
			case ALARM:
		    if((AlarmType&YJKSALARM)!= YJKSALARM)
				{
					LockStatus=ALARM;
					AlarmType&=~YJKSALARM;
			    AlarmType|=YJKSALARM;
			    EEPROM_OP((u8*)&LockStatus,LockStatus_ADDR,1,MODE_W);
			    EEPROM_OP((u8*)&AlarmType,AlarmType_ADDR,1,MODE_W);
          err=2;
          UARTWrite((u8*)"YJKS2\r\n",7,DEBUG_COM);//DEBUGGGGGGGGGGG
          AlarmCnt++;
			  }
				break;
		default:
		  break;
	  }
	  if(err)
	  {
	  	TT_Alarm_Status&=~ayjks;
			TT_Alarm_Status|=ayjks;
			RTC_OP(AlarmTimebuf,RTC_GETTIME_GB);
			EEPROM_OP(AlarmTimebuf,AlarmTime_ADDR,8,MODE_W);
			RFTxBuf[0]=NEW_CKALARM_EVENT;
	    GBRec_Center_FIFO(RFTxBuf,0,MODE_ALARM);
			GpsLocationOperate(&gpsx,RFTxBuf,DEVICE_LOCATION,TCPLINK);
	  }
}

/*******************************************************************************
* Function Name  : IOCheckHandler
* Description    : 检测报警脚
* Input          : None.
* Return         : None.
*******************************************************************************/
void IOCheckHandler(void)
{
	//锁杆断开报警
	if((HALL_Check_B)&&(HALL_Check_C ==0))
	{
		HallcheckBtick++;
		if(HallcheckBtick>4)
		{
			HallcheckBtick =0;
			LockOPEN_OP();
		}
	}
	else
		HallcheckBtick =0;

	//拆壳报警
	if((HALL_Check_B == 0)&&(HALL_Check_C))
	{
		HallcheckCtick++;
		if(HallcheckCtick>4)
		{
			HallcheckCtick =0;
			LockCHAIKE_OP();
		}
	}
	else
		HallcheckCtick =0;

	//非法开锁报警
//	if((HALL_Check_B)&&(HALL_Check_C ==0))
//	{
//		HallcheckBtick++;
//		if(HallcheckBtick>4)
//		{
//			HallcheckBtick =0;
//			LockYJKS_OP();
//		}
//	}
//	else
//		HallcheckBtick =0;

		//锁正常掉电
	if((HALL_Check_B)&&(HALL_Check_C))
	{
		checkpowofftick++;
		if(checkpowofftick>2)
		{
			checkpowofftick =0;
			if((LockStatus == UNSEAL)||(LockStatus == LOCKWAIT))
				POWER_CTR_OFF;
		}
	}
	else
		checkpowofftick =0;

	if(NeedSendAlarm)
	{
		NeedSendAlarm = 0;
		GpsLocationOperate(&gpsx,(u8*)0,DEVICE_LOCATION,TCPLINK);
	}
}

/*******************************************************************************
* Function Name  : VinsideHandler
* Description    : 电池电压采样
* Input          : None.
* Return         : None.
*******************************************************************************/
void  VinsideHandler(void)
{
	u16 adc_data;

	adc_data = DO_BatVol();
	printf("\r\n+Vbat:%d.%.3d V\r\n",adc_data/1000,adc_data%1000);
	Adc_Vdd = adc_data;

	if(adc_data<3550)
	{
		SysTick_Delay_Ms(1);
		adc_data = DO_BatVol();

//		printf("\r\n+Vbat:%d.%.3d V\r\n",adc_data/1000,adc_data%1000);
		Adc_Vdd = adc_data;
		if(adc_data<3550)
		{
			if(NeedSendAlarmCount>=2)
			{
				if((TT_Alarm_Status&aVDDLOW) == 0)
				{
					TT_Alarm_Status = TT_Alarm_Status|aVDDLOW;
					NeedSendAlarm = 1;
				}
				Vdd=0x30;
				GBVdd=0X61;//3.5V
			}
			NeedSendAlarmCount++;
			UARTWrite((u8*)"\r\n3333\r\n",8,DEBUG_COM);
		}
	}
	else
	{
		//避免电源波动//
		if(adc_data>=3550)
		{
			if(adc_data>3750)
			{
				TT_Alarm_Status&=~aVDDLOW;
				NeedSendAlarmCount = 0;
			}
			if(HadCut_MPW_BOLV != 0)
			{
				if(NetModuleSTATUS!=M35_KEEPOFF)
				{
					//3.7V如果已持续了500s，认为OK，可以开了//
					CanPWupModule_Cnt++;
					if(CanPWupModule_Cnt>=20) //20*25s
					{
						CanPWupModule_Cnt = 0;
						HadCut_MPW_BOLV = 0;
						TT_Alarm_Status&=~aVDDLOW;

						//因为模块关过，需要重新将程序做次初始化//
						ModuleNeedCutRST = 0;
						ModuleTempClosedCnt = 1;
						GPS_SW(1);
					}
				}
			}
			if(NetModuleSTATUS==LOWVDD_PWR_OFF)
			{
				NetModuleSTATUS = PWR_IN;
			}
		}
		if((adc_data>=3550)&&(adc_data<3750))
		{
			SysTick_Delay_Ms(2);
			adc_data = DO_BatVol();

//			printf("\r\n+Vbat:%d.%.3d V\r\n",adc_data/1000,adc_data%1000);
			UARTWrite((u8*)"\r\n1111\r\n",8,DEBUG_COM);
			Adc_Vdd = adc_data;
//			if((adc_data<3750))		//小于3.75V时不报警
//			{
//				if(NeedSendAlarmCount>=2)
//				{
//					Vdd=0x31;GBVdd=0X61;//3.5V
//					if((TT_Alarm_Status&aVDDLOW) == 0)
//					{
//						TT_Alarm_Status|=aVDDLOW;
//						NeedSendAlarm = 1;
//					}
//				}
//				NeedSendAlarmCount++;
//				UARTWrite((u8*)"\r\n3333\r\n",8,DEBUG_COM);
//			}
			if(NetModuleSTATUS==LOWVDD_PWR_OFF)
			{
				NetModuleSTATUS = PWR_IN;
			}
		}
		if((adc_data>=3750)&&(adc_data<3800))
		{
			Vdd=0x32;GBVdd=0X62;//3.85V
			NeedSendAlarmCount = 0;
			if(NetModuleSTATUS == LOWVDD_PWR_OFF)
				NetModuleSTATUS=PWR_IN;
		}
		if((adc_data>=3800)&&(adc_data<3900))
		{
			Vdd=0x33;GBVdd=0X62;//3.85V
			NeedSendAlarmCount = 0;
			if(NetModuleSTATUS==LOWVDD_PWR_OFF)
			{
				NetModuleSTATUS = PWR_IN;
			}
		}
		if((adc_data>=3900)&&(adc_data<4000))
		{
			Vdd=0x34;GBVdd=0X63;//4.10V
			NeedSendAlarmCount = 0;
			if(NetModuleSTATUS==LOWVDD_PWR_OFF)
			{
				NetModuleSTATUS = PWR_IN;
			}
		}
		if((adc_data>=4000)&&(adc_data<4100))
		{
			Vdd=0x35;GBVdd=0X63;//4.10V
			NeedSendAlarmCount = 0;
			if(NetModuleSTATUS==LOWVDD_PWR_OFF)
			{
				NetModuleSTATUS = PWR_IN;
			}
		}
		if((adc_data>=4100)&&(adc_data<4200))
		{
			Vdd=0x36; GBVdd=0X64;//4.20V
			NeedSendAlarmCount = 0;
			if(NetModuleSTATUS==LOWVDD_PWR_OFF)
			{
				NetModuleSTATUS = PWR_IN;
			}
		}
		if(adc_data>=4200)
		{
			Vdd=0x37; GBVdd=0X64;//4.20V
			NeedSendAlarmCount = 0;
			if(NetModuleSTATUS==LOWVDD_PWR_OFF)
			{
				NetModuleSTATUS = PWR_IN;
			}
		}
	}
	
	if(adc_data<3550)
	{
		Vdd=0x30;GBVdd=0X61;//3.5V  <3.55
	}
	else if((adc_data>=3550)&&(adc_data<3660))
	{
		Vdd=0x32;GBVdd=0X62;//3.5V  3.55-3.66
	}
	else if((adc_data>=3660)&&(adc_data<3800))
	{
		Vdd=0x32;GBVdd=0X62;//3.85V  3.66-3.8
	}
	else if((adc_data>=3800)&&(adc_data<3900))
	{
		Vdd=0x33;GBVdd=0X62;//3.85V  3.8-3.9
	}
	else if((adc_data>=3900)&&(adc_data<4000))
	{
		Vdd=0x34;GBVdd=0X63;//4.10V  3.9-4.0
	}
	else if((adc_data>=4000)&&(adc_data<4100))
	{
		Vdd=0x35;GBVdd=0X63;//4.10V  4.0-4.1
	}
	else if((adc_data>=4100)&&(adc_data<4200))
	{
		Vdd=0x36; GBVdd=0X64;//4.20V 4.1-4.2
	}
	else if(adc_data>=4200)
	{
		Vdd=0x37; GBVdd=0X64;//4.20V >4.2
	}
	
	if(adc_data<3550)
		vddlow_flag=1;
	else
		vddlow_flag=0;
}

/*******************************************************************************
* Function Name  : PsecondEventHandler
* Description    : PsecondEventHandler    250ms执行一次
* Input          : None.
* Return         : None.
*******************************************************************************/
void PsecondEventHandler(void)
{
	if(Tick_250ms)
	{
		if(delay1stick<100)
			delay1stick++;
		
		Tick_250ms = 0;
#if NEED_GPS
		GpsTimerHandler();
#endif
		IOCheckHandler();
		
		GPS_disp_tick++;
		if(GPS_disp_tick>=200)//50s
		{
			GPS_disp_tick=0;
			GPS_disp_flag=GPS_Valid_flag;
		}
		
		if(RF_tick)
		{
			RF_LP_flag=0;
			RF_tick--;
			if(RF_tick==0)
			{
				RF_LP_flag=1;
			}
		}
#if NEED_GPRS
		if((NetModuleSTATUS!=M35_KEEPOFF)&&(NetModuleSTATUS!=LOWVDD_PWR_OFF))
		{
			if(Get_Gprs_DTR() == 0)
			{
				Module_TimerHandler();
				if((TCPIP_been_OK&0x03)== 0x03)
				{
					Lowpowertick ++;
					if(Lowpowertick>= 20)  // 20*250=5秒
					{
						GPRS_LP_flag=1;
						Lowpowertick = 0;
						UARTWrite((u8*)"\r\ngprszzz\r\n",11,0);
						GPRS_DTR_H;
					}
				}
			}
			else if(Get_Gprs_DTR() == 1)
			{
				if(((TCPIP_been_OK&0x03)!= 0x03)||(Readyrstcnt||GPRSReConnectCnt))
				{
					GPRS_LP_flag=0;
					GPRS_DTR_L;
					UARTWrite((u8*)"\r\ngprsemwake\r\n",14,0);
				}
			}
			//Module_TimerHandler();
			TCPIPTimerHandler();	//定时发送心跳包
			if(Send2CCBuf[0])
			{
			//Send2CCBuf[0]                           数据长度
			//Send2CCBuf[1] Send2CCBuf[2]    消息ID
			//Send2CCBuf[3].....                       数据
				NewPacket_KY((u8*)&Send2CCBuf[3],Send2CCBuf[0],(((u16)Send2CCBuf[1]<<8)+Send2CCBuf[2]),TCPLINK);
				Send2CCBuf[0]=0;
			}
		}
#endif
		AD0PS_cnt++;
		if(AD0PS_cnt>=40)     //10s检测vcc
		{
			AD0PS_cnt=0;
			if(Needresetflag[2] != 0x55)
			{
				UARTWrite((u8*)"AT+IPR=57600\r\n",14,GPRS_COM);
				SysTick_Delay_Ms(5);
				UARTWrite((u8*)"AT&W\r\n",6,GPRS_COM);
				SysTick_Delay_Ms(5);
				UARTWrite((u8*)"AT&W\r\n",6,GPRS_COM);
				SysTick_Delay_Ms(5);
				UARTWrite((u8 *)"AT+QNITZ=1\r\n",12,GPRS_COM);
				SysTick_Delay_Ms(5);
				UARTWrite((u8 *)"AT+CTZU=3\r\n",11,GPRS_COM);
			}
			if(Tcpsendflag == 1)
			{
				tcpsendtick++;
				qisacktick++;
				if(tcpsendtick >= 36)
				{
					tcpsendtick = 0;
					ModuleFlagsIni();
					NETFlagsIni();
					UARTWrite((u8 *)"EM_R\r\n",6,0);//debugggg
					NetModulePowerOP(EM_PWR_OFF);//关机//
				}
				if(qisacktick >= 36)
				{
					qisacktick = 0;
					ModuleFlagsIni();
					NETFlagsIni();
					UARTWrite((u8 *)"EM_R\r\n",6,0);//debugggg
					NetModulePowerOP(EM_PWR_OFF);//关机//
				}
			}
//			RFM_Ini((u8*)LockCHcfg);
		}
		VinsCount++;
		if(VinsCount >= 240)   //   250ms*120 = 30s
		{
			VinsCount = 0;
			VinsideHandler();
		}
	}
}


/*******************************************************************************
* Function Name  : CheckRFIDForSleep
* Description    : CheckRFIDForSleep
* Input          : None.
* Return         : None.
*******************************************************************************/
void CheckForSleep(void)
{
	if((GPRS_LP_flag)&&(GPS_LP_flag)&&(RF_LP_flag)&&(KEY_LP_flag==0))//&&(RF_LP_flag)
	{
		CheckLowPower=1;
	}
}


