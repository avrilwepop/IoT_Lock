#include "rfid.h"
#include "TTLM.h"
#include "gbprotocol.h"
#include "BSprotocol.h"
#include "crcencoder.h"
#include "gprs.h"
#include "common.h"
#include "lp_mode.h"

u8  LockCHcfg[10] =					//配置命令
{
	0x78,											//CH_NO,配置频段在423MHZ
	0x0c,											//输出功率为+10db,重发，节电为正常模式 0x0010 1100
	0x44,											//地址宽度设置，为4字节
	0x20,0x20,								//接收发送有效数据长度为32字节
	0xcc,0xcc,0xcc,0xcc,			//接收地址
	0x5c											//CRC充许，8位CRC校验，外部时钟信号不使能，16M晶振
};
//u8  LockCHcfg[10] =					//配置命令
//{
//	0x78,											//CH_NO,配置频段在423MHZ
//	0x0c,											//输出功率为+10db,不重发，节电为正常模式 0x0010 1100
//	0x44,											//地址宽度设置，为4字节
//	0x20,0x20,								//接收发送有效数据长度为32字节
//	0xcc,0xcc,0xcc,0xcc,			//接收地址
//	0x5c											//CRC允许，8位CRC校验，外部时钟信号不使能，16M晶振
//};
u8  RfRxReady=0;
u8  RfRxPktLen=0;
u8 	CmdSerial = 1;
u8  RFsn0;
u8  RFsnBack;
u8  TempTxBuf[33];
u8  RfRxBuf[100];
u8 RFRxBuf[100];
u8 NEWcmd;
u8 OPResult;
u8 LastOPTime[9];
u8 RecSeq;
u8 RFSendLen;
extern u8 keyopresult;
u8 RF_tick=0;

/*******************************************************************************
* Function Name  : NRF905_IOinit
* Description    : Configures NRF905
* Input          : None.
* Return         : None.
*******************************************************************************/
void NRF905_IOinit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	SPI_RF_MOSI_H;

	//MISO
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd =GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	//SPI_RF_SCK
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	SPI_RF_SCK_L;

	/***TRX_CE  ***/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	/***TXEN ***/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	/*** PWR_UP***/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	/***CSN ***/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	CSN_H;
	
	Set_StandBy();
	
	//DR PC9(上升沿触发)
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode =GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd =GPIO_PuPd_DOWN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource9);

	EXTI_InitStructure.EXTI_Line = EXTI_Line9;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  		//上升沿触发
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	EXTI_ClearITPendingBit(EXTI_Line9);  //清除EXTI0线路挂起位
	EXTI_ClearFlag(EXTI_Line9);          //清楚EXTI9线路挂起标志位
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; //从优先级0级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //使能外部中断通道
	NVIC_Init(&NVIC_InitStructure);
}

/*******************************************************************************
* Function Name  : RFM_Ini
* Description    : RFM_Ini
* Input          : rfcfg.
* Return         : None.
*******************************************************************************/
void RFM_Ini(u8 * rfcfg)
{
	RFINT_en();
	Set_StandBy();
	SysTick_Delay_Ms(2);
	Config905((u8*)rfcfg);
	SysTick_Delay_Ms(1);
	SetRxMode();
}
/*******************************************************************************
* Function Name  : RFINT_en
* Description    : RFINT_en
* Input          : None.
* Return         : None.
*******************************************************************************/
void RFINT_en(void)
{
	NVIC_EnableIRQ(EXTI9_5_IRQn);
}
/*******************************************************************************
* Function Name  : RFINT_off
* Description    : RFINT_off
* Input          : None.
* Return         : None.
*******************************************************************************/
void RFINT_off(void)
{
	NVIC_DisableIRQ(EXTI9_5_IRQn);
}

void Set_StandBy(void)
{
	RFPWR_UP_H;//Standby和SPI编程 
	TRX_CE_L;
	TXEN_L;
}
	
/*******************************************************************************
* Function Name  : SetRxMode
* Description    : Config905 Receive
* Input          : NONE.
* Return         : NONE.
*******************************************************************************/
void SetRxMode(void)
{
 	RFPWR_UP_H;
	TXEN_L;
	TRX_CE_H;
	SysTick_Delay_Ms(1);
}

/*******************************************************************************
* Function Name  : SetTxMode
* Description    : Config905 Send
* Input          : NONE.
* Return         : NONE.
*******************************************************************************/
void SetTxMode(void)
{
	RFPWR_UP_H;
	TXEN_H;
	TRX_CE_L;
	SysTick_Delay_Ms(1);
}

/*******************************************************************************
* Function Name  : SetOFFMode
* Description    : Config905 POWER OFF
* Input          : NONE.
* Return         : NONE.
*******************************************************************************/
void SetOFFMode(void)
{
	NVIC_DisableIRQ(EXTI9_5_IRQn);
  RFPWR_UP_L;
	SysTick_Delay_Ms(1);
}

/*******************************************************************************
* Function Name  : SPI_Send
* Description    : SPI_Send
* Input          : None.
* Return         : None.
*******************************************************************************/
void SPI_Send(u8 byte)
{
	static u8  DATA_BUF;
	u8 i;

	DATA_BUF=byte;
	for(i=0;i<8;i++)
	{
		if(DATA_BUF& 0x80)
		{
			SPI_RF_MOSI_H;
		}
		else
		{
			SPI_RF_MOSI_L;
		}
		SPI_RF_SCK_H;
		DATA_BUF<<=1;
		SPI_RF_SCK_L;
	}
}
/*******************************************************************************
* Function Name  : SPI_Rcv
* Description    : SPI_Rcv
* Input          : None.
* Return         : None.
*******************************************************************************/
u8 SPI_Rcv(void)
{
	u8 i;
	u8 data=0;
	for(i=0;i<8;i++)
	{
		data<<=1;
		SPI_RF_SCK_H;
		if(SPI_RF_MISO==1)
		{
			data|=0x01;
		}
		else
		{
			data &= 0xfe;
		}
		SPI_RF_SCK_L;
	}
	return data;
}

/*******************************************************************************
* Function Name  : Config905
* Description    : Config905
* Input          : NONE.
* Return         : NONE.
*******************************************************************************/
void Config905(u8 *rfcfgbuf)
{
	u8 i;
	CSN_L;
	SPI_Send(WC);
	for(i=0;i<10;i++)
	{
		SPI_Send(*(rfcfgbuf+i));
	}
	CSN_H;
}

/*******************************************************************************
* Function Name  : RdRfBuf
* Description    : RdRfBuf
* Input          : None.
* Return         : None.
*******************************************************************************/
u16 RdRfBuf(u8 * buf)
{
	RFsnBack=RFsn0;
	memcopy(buf,RfRxBuf,RfRxPktLen);
	RfRxReady=0;
	return RfRxPktLen;
}

/*******************************************************************************
* Function Name  : ReadRxData
* Description    : ReadRxData
* Input          : None.
* Return         : None.
*******************************************************************************/
void  ReadRxData(void)
{
	u8 i,var,sn;
	static u8 rfdataindex = 0;

	TRX_CE_L;
	CSN_L;
	SPI_Send(RRP);            //准备读取接收到的数据

	var = SPI_Rcv();			//读取包头
	if(var>0x10)
	{
		sn = SPI_Rcv();
		if(((var&0x0f)==1 )&&(rfdataindex == 0))
		{
			RFsn0=sn;
			rfdataindex = 1;
			RfRxReady=0;
			RfRxPktLen = 0;
		}

		if(sn==RFsn0)
		{
			if((var&0x0f) == rfdataindex)    //序号对应...
			{
				for(i=0;i<30;i++)
				{
					RfRxBuf[i+((var&0x0f)-1)*30]=SPI_Rcv();
				}
				RfRxPktLen+=30;
				rfdataindex++;
				if((var&0xf0)>>4 == (var&0x0f))
				{
					RfRxReady=GB_MODE;
					rfdataindex = 0;
				}
			}
			else
			{
				RfRxPktLen = 0;
				rfdataindex = 0;
			}
		}
		else
		{
			rfdataindex = 0;
			RfRxPktLen = 0;
			for(i=0;i<30;i++)
			{
				SPI_Rcv();
			}
		}
	}
	else
	{
		for(i=0;i<31;i++)
		{
			SPI_Rcv();
		}
	}
	CSN_H;
	TRX_CE_L;
	SetRxMode();
}
/*******************************************************************************
* Function Name  : TxData
* Description    : TxData
* Input          : None.
* Return         : None.
******************************************************************************/
void TxData(void)
{
	 u8  i;
	 SetTxMode();

	CSN_L;

	SPI_Send(WTP);
	for (i=0;i<32;i++)
	{
		SPI_Send(TempTxBuf[i]);
	}
	CSN_H;
	SysTick_Delay_Ms(1);
	CSN_L;
	SPI_Send(WTA);
	for (i=0;i<4;i++)
	{
		SPI_Send(LockCHcfg[i+5]);
	}
	CSN_H;

	TRX_CE_H;				// Set TRX_CE high,start Tx data transmission
	SysTick_Delay_Ms(6);
	TRX_CE_L;				//Set TRX_CE low
	SysTick_Delay_Ms(10);
}

/*******************************************************************************
* Function Name  : GB_LockSend
* Description    : GB_LockSend
* Input          : None.
* Return         : None.
*******************************************************************************/
void GB_LockSend(u8 * buf,u8 len)
{
	  u8 total_pktN,endlen,i;

    SysTick_Delay_Ms(4);
    RFINT_off();

    total_pktN=len/30;
    endlen=len%30;
    if(endlen) { total_pktN++; }

    TempTxBuf[0]=(total_pktN<<4)&0xf0;
    TempTxBuf[1]=RFsnBack;//rand();

    for(i=0;i<total_pktN;i++)
    {
    	TempTxBuf[0]++;
    	memcopy(TempTxBuf+2,buf+i*30,30);
		if((i==(total_pktN-1))&&(endlen))
    	{
    		memREset(TempTxBuf+2+endlen,0x30,30-endlen);
    	}
    	//UARTWrite(TempTxBuf,32,0);
      TxData();
    }
    SetRxMode();
    RFINT_en();
}

/*******************************************************************************
* Function Name  : RF_Process
* Description    : RF_Process
* Input          : None.
* Return         : None.
*******************************************************************************/
void RF_Process(void)
{
	u8 rfdatalen;

	if(RfRxReady == GB_MODE)
	{
		RFINT_off();
		rfdatalen = RdRfBuf(RFRxBuf);
		RfRxPktLen = 0;
		RFINT_en();
		
		GB_RFRxDataOP(rfdatalen);
	}
}
/*******************************************************************************
* Function Name  : GB_RFRxDataOP
* Description    : RF数据收发处理函数
* Input          : None.
* Return         : None.
*******************************************************************************/
void GB_RFRxDataOP(u8 lenth)
{
	u8 cmd,recmd;
	u8 i,j,crcor;
	u16 crc16,len;
	u8 Bakkeybuf[10];//u8 Bakkeybuf[256];
	u8 keybuf1[6],keybuf2[6];

	if(RFRxBuf[0]==XYID)//7b 老国标协议
	{
		if(memCpare(RFRxBuf+1,GB_LOCKIDbuf,8))
		{
			//UARTWrite((u8*)"ACK_RCV\r\n",9,DEBUG_COM);
			return;
		}
		else if(RFRxBuf[1]&bJAM)
		{
			return; //不加密...
		}
		if(RFRxBuf[1]&bUP)
		{
			//UARTWrite((u8*)"UP\r\n",4,0);
			return;
		}
		if(RFRxBuf[1]&bP2P)
		{
			if(!memCpare(RFRxBuf+2,GB_LOCKIDbuf,8))
			{
				return;
			}
		}

		GB_RFACK();

		i=RFRxBuf[19]+20;
		crc16=((RFRxBuf[i]<<8)&0xff00)|RFRxBuf[i+1];
		if(crc16!=MakeCRC16(RFRxBuf,i))
		{
			//UARTWrite((u8*)"CRC ERR1\r\n",10,0);
			return ;
		}
		if((RFRxBuf[0] ==XYID)&&((RFRxBuf[1]&bJAM) == 0)&&((RFRxBuf[1]&bP2P) ==0)&&(RFRxBuf[18] == GB_CHECK))
		{
			UARTWrite((u8*)"RF_ACK\r\n",8,DEBUG_COM);
			return;
		}

		SysTick_Delay_Ms(5);
		cmd=RFRxBuf[18];
		NEWcmd=0;
		//SendData2TestCOM((u8*)RFRxBuf,RFRxBuf[19]+22,0);//debugggggggggg//

		switch(cmd)
		{
			case PRELOCK:
			case GB_LOCK:
				if(1)
				{
					SendData2TestCOM((u8 *)&LockStatus,1,0);
					if(cmd==PRELOCK)
					{
						recmd=PRELOCK_BACK;
					}
					else
					{
						recmd=GB_LOCK_BACK;
					}
					if(0==memCpare(LastOPTime,RFRxBuf+30,8) )
					{
						NEWcmd=1;
						memcopy(LastOPTime,RFRxBuf+30,8);
					}
					if(Vdd<0x32)
					{
						OPResult=0X00;
						OPResult=(bNOT_OK|bVddLOW);
						GB_RFTxDatPacketSend(recmd,1,1);
						return;
					}
					switch(LockStatus)
					{
						//case LOCKWAIT:
						//case PWON:
						//case UNSEAL:
						//break;

						case SEAL:
						case EXALARM:
							OPResult=0x00;
							if(CmdSerial!=RFRxBuf[37])
							{
								OPResult=bNOT_OK|bAGAIN;
								MotoDriver(bLOCK);
							}
							GB_RFTxDatPacketSend(recmd,1,1);
							AlarmType=0;
							EEPROM_OP((u8*)&AlarmType,AlarmType_ADDR,1,MODE_W);
							break;

						case ALARM:
							if((AlarmType&OPENALARM)==OPENALARM)
								{ OPResult=bNOT_OK|bOUT;}//锁杆断开OPENALARM//RFTxBuf[dindex-1]=1;
							else  if((AlarmType&CUTALARM)==CUTALARM)
								{ OPResult=bNOT_OK|bCKBJ;}//非法拆开RFTxBuf[dindex-1]=1;
							else  if((AlarmType&CKALARM)==CKALARM)
								{ OPResult=bNOT_OK|0x40;}//
							else  if((AlarmType&YJKSALARM)==YJKSALARM)
								{ OPResult=bNOT_OK|0x80;}//
							GB_RFTxDatPacketSend(recmd,1,1);
							break;
						default:
							if(HALL_Check_B == 0)  //都闭合
							{
								MotoDriver(bLOCK);
								LockStatus=SEAL;
								OPResult=0x00;
								AlarmType=0;
								GB_RFTxDatPacketSend(recmd,1,1);

								EEPROM_OP((u8*)&AlarmType,AlarmType_ADDR,1,MODE_W);

								memcopy(SecKeyBuf,RFRxBuf+20,10);
								EEPROM_OP(SecKeyBuf,SecKey_ADDR,10,MODE_W);
								SysTick_Delay_Ms(1);
								EEPROM_OP(SecKeyBuf,SecKey_ADDR2,10,MODE_W);

								CmdSerial=RFRxBuf[37];
								EEPROM_OP((u8*)&LockStatus,LockStatus_ADDR,1,MODE_W);
								Bakkeybuf[0]=GB_LOCK_EVENT;
								GBRec_FIFO(Bakkeybuf,1,MODE_W);
							}
							else
							{
								OPResult=bNOT_OK|bOUT;
								GB_RFTxDatPacketSend(recmd,1,1);
							}

							break;
					}
				}
				break;

			case GB_UNLOCK:
				if(1)
				{
					//RTC_OP(RFRxBuf+30,ADJUST_RTC_DTIME_GB);
					if(0== memCpare(LastOPTime,RFRxBuf+30,8) )
					{
						NEWcmd=1;
						memcopy(LastOPTime,RFRxBuf+30,8);
					}
					switch(LockStatus)
					{
						case LOCKWAIT:
							OPResult=bNOT_OK|bNOTSEAL;
							GB_RFTxDatPacketSend(GB_UNLOCK_BACK,1,1);
							ReUnLock();
							break;

						case SEAL:
						case EXALARM:
							if(0==KEY_VN((u8)(bNOT_OK|bKEYERR),GB_UNLOCK_BACK))
							{
							return;
							}
							MotoDriver(bUnLOCK);

							OPResult=0X00;
							AlarmCnt=0;
							AlarmType=0;
							GB_RFTxDatPacketSend(GB_UNLOCK_BACK,1,2);

							CmdSerial=RFRxBuf[37];
							LockStatus=UNSEAL;
							EEPROM_OP((u8*)&LockStatus,LockStatus_ADDR,1,MODE_W);
							EEPROM_OP((u8*)&AlarmType,AlarmType_ADDR,1,MODE_W);
							Bakkeybuf[0]=GB_UNLOCK_EVENT;
							GBRec_FIFO(Bakkeybuf,0,MODE_W);


						/////////
							break;

						case ALARM:
							OPResult=bNOT_OK|bALARM;
							GB_RFTxDatPacketSend(GB_UNLOCK_BACK,1,1);
							break;

						case UNSEAL:
							OPResult=0X00;
							if(CmdSerial==RFRxBuf[37])
							{
								GB_RFTxDatPacketSend(GB_UNLOCK_BACK,1,1);
							}
							else
							{
								OPResult=bNOT_OK|bAGAIN;
								GB_RFTxDatPacketSend(GB_UNLOCK_BACK,1,1);
								ReUnLock();
							}
							break;

						default:
							ReUnLock();
							break;
					}
				}
				break;

			case GB_CLR_ALARM:
				if(1)
				{
					if(0==KEY_VN((u8)bKEYERR,GB_CLR_ALARM_BACK))
					{
						return;
					}
					switch(LockStatus)
					{
						case ALARM:
							LockStatus=EXALARM;
							TT_Alarm_Status = 0;	//20180428,解决手持机解警后，后台依然报警
						
							OPResult=bCLR_OK;
							GB_RFTxDatPacketSend(GB_CLR_ALARM_BACK,1,1);

							CmdSerial=RFRxBuf[37];
							EEPROM_OP((u8*)&LockStatus,LockStatus_ADDR,1,MODE_W);
							Bakkeybuf[0]=GB_CLRALARM_EVENT;
							GBRec_FIFO(Bakkeybuf,0,MODE_W);
							break;

						case EXALARM:
							if(CmdSerial==RFRxBuf[37])
							{
								OPResult=bCLR_OK;
							}
							else
							{
								OPResult=bNO_ALARM;
							}
							GB_RFTxDatPacketSend(GB_CLR_ALARM_BACK,1,1);
							break;

							default:
								OPResult=bNO_ALARM;
								GB_RFTxDatPacketSend(GB_CLR_ALARM_BACK,1,1);
								break;
						}
				}
				break;
			case GB_CHECK:
				if(1)
				{
					//RTC_OP(RFRxBuf+30,ADJUST_RTC_DTIME_GB);
				switch(LockStatus)
				{
					case SEAL:
						OPResult=0x01;
						break;

					case ALARM:
					case EXALARM:
						if(MoToStatus)
							{OPResult=0x02;}
						else
							{OPResult=0x03; }
						break;
					default:
						OPResult = 0x04;
						break;
					}
					GB_RFTxDatPacketSend(GB_CHECK_BACK,1,1);
				}
				break;

			case GB_RD_EVENTDATA:
				if(1)
				{
					if(!memCpare(RFRxBuf+20,SecKeyBuf,10))
					{
						EEPROM_OP(Bakkeybuf,SecKey_ADDR2,10,MODE_R);
						EEPROM_OP(SecKeyBuf,SecKey_ADDR,10,MODE_R);
						if(!memCpare(RFRxBuf+20,Bakkeybuf,10))
						{
							if(!memCpare(RFRxBuf+20,(u8*)WNMM,10))
							{
								OPResult=0x04;
								GB_RFTxDatPacketSend(GB_RD_EVENTDATA_BACK,1,1);
								return;
							}
						}
					}
					OPResult = 0x01;
					RecSeq = RFRxBuf[30];
					if(RecSeq == 0xff)
					{
						for(RecSeq=1;RecSeq<=3;RecSeq++)
						{
							for(j = 0 ;j< 200;j++)
							{
								SysTick_Delay_Ms(5);

							}
							GB_RFTxDatPacketSend(GB_RD_EVENTDATA_BACK,1,0);
						}
					}
					else
					{
						GB_RFTxDatPacketSend(GB_RD_EVENTDATA_BACK,1,1);
					}
				}
				break;

			case GB_WR_CID:
				break;

			case GB_RD_CID:

				break;

			case GB_DL_READER_PKEY:
				break;

			case RZ_READER:
				break;
			case CERTIFI_ISSU:	//制发
				break;

			case CERTIFI_INIT:	//复位
				break;

			case MADE_AQZS: 	//制发
				break;

			case RESET_SAFEIC:
				break;
			case GET_READERID:
				GB_RFTxDatPacketSend(GET_READERID_BACK,1,1);
				break;
			case SET_READERID:
				memcopy(GB_LOCKIDbuf,RFRxBuf+2,8);
				EEPROM_OP((u8*)GB_LOCKIDbuf,GB_LOCKID_ADDR,8,MODE_W);

				GB_RFTxDatPacketSend(SET_READERID_BACK,1,1);
				SeqCN=0;
				EEPROM_OP((u8*)&SeqCN,SeqCN_ADDR,4,MODE_W);
				TotalRecNum1 = 0;
				EEPROM_OP((u8*)&TotalRecNum1,TotalRecNum_ADDR,1,MODE_W);
				RecordIndex = 0;
				EEPROM_OP((u8*)&RecordIndex,RECORDINDEX_ADDR,2,MODE_W);

				break;
			case SET_GATE:
				break;
			case SET_FHM://EW//
				break;
			case SET_RFADD:
				break;
			case GET_RFADD:
				break;
			case GET_GATE:
				break;
			case SET_GPS_MODE:
				break;
			case GB_GPSGSM_OFF:
			case GB_GPSGSM_ON:
			case GB_GPSOFF_GSMON:
				break;

			case GB_RD_CHECKFLAG:
			case GB_WR_CHECKFLAG:
				break;


			case GB_SetSIMNumber:
				break;
			default:
				break;
		}

	}
	else if((RFRxBuf[0]==HEAD)&&(RFRxBuf[1]==HEAD))//新协议
	{
		if(!memCpare(RFRxBuf+5,DevParameters.snid,6))
		{
			return;
		}
		len = ((u16)RFRxBuf[2]<<8)+(u16)RFRxBuf[3];
		crcor = Do_XOR(RFRxBuf,len-1);
		if(crcor!= RFRxBuf[len-1])
			return;

		SysTick_Delay_Ms(5);
		cmd=RFRxBuf[4];
		switch(cmd)
		{
			case NEWRF_LOCK:
				if(1)
				{
					SendData2TestCOM((u8 *)&LockStatus,1,0);

					recmd=NEWRF_LOCK_BACK;

					if(Vdd<0x32)
					{
						OPResult=0X00;
						OPResult=(bNOT_OK|bVddLOW);
						NEW_RFTxDatPacketSend(recmd,1,1);
						return;
					}
					switch(LockStatus)
					{

						case SEAL:
						case EXALARM:

							OPResult=bNOT_OK|bAGAIN;
								//MotoDriver(bLOCK);
							NEW_RFTxDatPacketSend(recmd,1,1);
							AlarmType=0;
							EEPROM_OP((u8*)&AlarmType,AlarmType_ADDR,1,MODE_W);
							break;
						case ALARM:
							if((AlarmType&OPENALARM)==OPENALARM)
								{ OPResult=bNOT_OK|bOUT;}//锁杆断开OPENALARM//RFTxBuf[dindex-1]=1;
							else  if((AlarmType&CUTALARM)==CUTALARM)
								{ OPResult=bNOT_OK|bCKBJ;}//非法拆开RFTxBuf[dindex-1]=1;
							else  if((AlarmType&CKALARM)==CKALARM)
								{ OPResult=bNOT_OK|0x40;}//
							else  if((AlarmType&YJKSALARM)==YJKSALARM)
								{ OPResult=bNOT_OK|0x80;}//
							NEW_RFTxDatPacketSend(recmd,1,1);
							break;
						default:
							if(HALL_Check_B == 0)  //都闭合
							{

							MotoDriver(bLOCK);
							LockStatus=SEAL;
							OPResult=0x00;
							AlarmType=0;
							NEW_RFTxDatPacketSend(recmd,1,1);

							EEPROM_OP((u8*)&AlarmType,AlarmType_ADDR,1,MODE_W);
							SeqCN++;
							EEPROM_OP((u8*)&SeqCN,SeqCN_ADDR,4,MODE_W);

							memcopy(SecKeyBuf6,RFRxBuf+12,6);
							EEPROM_OP(SecKeyBuf6,SecKey6_ADDR,6,MODE_W);
							SysTick_Delay_Ms(1);
							EEPROM_OP(SecKeyBuf6,SecKey6_ADDR2,6,MODE_W);

							EEPROM_OP((u8*)&LockStatus,LockStatus_ADDR,1,MODE_W);

							//Bakkeybuf[0]=NEW_LOCK_RFID_EVENT;
							//GBRec_Center_FIFO(Bakkeybuf,1,MODE_W);
							}
							else
							{
								OPResult=bNOT_OK|bOUT;
								NEW_RFTxDatPacketSend(recmd,1,1);
							}
							break;
					}
				}
				break;

			case NEWRF_UNLOCK:
				if(1)
				{
					recmd=NEWRF_UNLOCK_BACK;
					switch(LockStatus)
					{
						case LOCKWAIT:
							OPResult=bNOT_OK|bNOTSEAL;
							NEW_RFTxDatPacketSend(recmd,1,1);
							ReUnLock();
							break;

						case SEAL:
						case EXALARM:
							EEPROM_OP(keybuf1,SecKey6_ADDR,6,MODE_R);
							EEPROM_OP(keybuf2,SecKey6_ADDR2,6,MODE_R);
							if(RFRxBuf[11] ==0x00)  //
							{
								if(!memCpare(RFRxBuf+12,keybuf1,6))
								{
									if(!memCpare(RFRxBuf+12,keybuf2,6))
									{
											OPResult = bNOT_OK|bKEYERR;
											NEW_RFTxDatPacketSend(recmd,1,2);
											Bakkeybuf[0]=NEW_KEYERR_EVENT;
											GBRec_Center_FIFO(Bakkeybuf,1,MODE_W);
									}
								}
							}
							else if((RFRxBuf[11] == 0x01)||((RFRxBuf[11] ==0x00)&&((memCpare(RFRxBuf+12,keybuf1,6))||(memCpare(RFRxBuf+12,keybuf2,6)))))
							{
								MotoDriver(bUnLOCK);
								OPResult=0x00;
								AlarmCnt=0;
								AlarmType=0;

								NEW_RFTxDatPacketSend(recmd,1,2);
								LockStatus=UNSEAL;
								EEPROM_OP((u8*)&LockStatus,LockStatus_ADDR,1,MODE_W);

								Bakkeybuf[0]=NEW_UNLOCK_RFID_EVENT;
								GBRec_Center_FIFO(Bakkeybuf,1,MODE_W);

								EEPROM_OP((u8*)&AlarmType,AlarmType_ADDR,1,MODE_W);
								SeqCN++;
								EEPROM_OP((u8*)&SeqCN,SeqCN_ADDR,4,MODE_W);
							}
							break;

						case ALARM:
							OPResult=bNOT_OK|bALARM;
							NEW_RFTxDatPacketSend(recmd,1,2);
							break;

						case UNSEAL:

							OPResult=bNOT_OK|bAGAIN;
							NEW_RFTxDatPacketSend(recmd,1,2);
							ReUnLock();
							break;

						default:
							ReUnLock();
							break;
					}
				}
				break;

			case NEWRF_CLR_ALARM:
				if(1)
				{
					recmd = NEWRF_CLR_ALARM_BACK;

					switch (LockStatus)
					{
						case ALARM:

							LockStatus=EXALARM;
							OPResult=0x00;
							NEW_RFTxDatPacketSend(recmd,1,2);
							EEPROM_OP((u8*)&LockStatus,LockStatus_ADDR,1,MODE_W);
							Bakkeybuf[0]=NEW_CLRALARM_EVENT;
							GBRec_Center_FIFO(Bakkeybuf,1,MODE_W);

							break;
						case EXALARM:
							OPResult=bNO_ALARM;
							NEW_RFTxDatPacketSend(recmd,1,2);
							break;
						default:
							OPResult=bNO_ALARM;
							NEW_RFTxDatPacketSend(recmd,1,2);
							break;
					}
				}
				break;
			case NEWRF_CHECK:
				if(1)
				{
					recmd = NEWRF_CHECK_BACK;

					switch(LockStatus)
					{
						case SEAL:
							OPResult=0x01;
							break;

						case ALARM:
						case EXALARM:
							if(MoToStatus)
								{OPResult=0x02;}
							else
								{OPResult=0x03; }
							break;
						default:
							OPResult = 0x04;
							break;
						}
					NEW_RFTxDatPacketSend(recmd,1,1);
				}
				break;
			case NEWRF_RD_EVENTDATA:
				if(1)
				{
					recmd = NEWRF_RD_EVENTDATA_BACK;

					OPResult = 0x01;
					RecSeq = RFRxBuf[11];
					if(RecSeq == 0xff)
					{
						for(RecSeq=1;RecSeq<=3;RecSeq++)
						{
							for(j = 0 ;j< 200;j++)
							{
								SysTick_Delay_Ms(5);

							}
							NEW_RFTxDatPacketSend(recmd,1,1);
						}
					}
					else
					{
						NEW_RFTxDatPacketSend(recmd,1,1);
					}
				}
				break;
			default:
				break;
		}
	}

}
/*******************************************************************************
* Function Name  : CenterCmdOP
* Description    : CenterCmdOP
* Input          : None.
* Return         : None.
*******************************************************************************/
void CenterCmdOP(u8 * buf,u8 lenth,u8* Serverindex,u16 cmd,u8 op)
{
	u16 cmdback;
	u8 tmp[30],index=0;
	u8 Bakkeybuf[256];
	u8 keybuf1[6],keybuf2[6];

	if(op == SERVER_OPR)
	{
		tmp[index] = Serverindex[0];index++;
		tmp[index] = Serverindex[1];index++;//应答流水号填充
		switch(cmd)
		{
			case DEVICE_LOCK:
				SendData2TestCOM((u8 *)&LockStatus,1,0);
				cmdback = DEVICE_LOCK_BACK;
				if(Vdd<0x32)//低于3.5v
				{
					OPResult=(bNOT_OK|bVddLOW);
				}
				else
				{
					switch(LockStatus)
					{
						case SEAL:				//0X40
						case EXALARM:		//0X90
							AlarmType=0;
							EEPROM_OP((u8*)&AlarmType,AlarmType_ADDR,1,MODE_W);
							OPResult=bNOT_OK|bAGAIN;
							MotoDriver(bLOCK);
							break;
						case ALARM:			//0X70
							if((AlarmType&OPENALARM)==OPENALARM)
							{ OPResult=bNOT_OK|bOUT;}//锁杆断开OPENALARM//RFTxBuf[dindex-1]=1;
							else  if((AlarmType&CUTALARM)==CUTALARM)
							{ OPResult=bNOT_OK|bCKBJ;}//非法拆开RFTxBuf[dindex-1]=1;
								else if((AlarmType&CKALARM)==CKALARM)
									{ OPResult=bNOT_OK|0x40;}
								else  if((AlarmType&YJKSALARM)==YJKSALARM)
									{ OPResult=bNOT_OK|0x80;}
							break;

						default:
							if(HALL_Check_B == 0)  //都闭合
							{
								MotoDriver(bLOCK);
								LockStatus=SEAL;
								OPResult=0x00;
								AlarmType=0;
								EEPROM_OP((u8*)&AlarmType,AlarmType_ADDR,1,MODE_W);
								SeqCN++;
								EEPROM_OP((u8*)&SeqCN,SeqCN_ADDR,4,MODE_W);

								memcopy(SecKeyBuf6,buf+1,6);
								EEPROM_OP(SecKeyBuf6,SecKey6_ADDR,6,MODE_W);
								SysTick_Delay_Ms(2);
								EEPROM_OP(SecKeyBuf6,SecKey6_ADDR2,6,MODE_W);
								SysTick_Delay_Ms(2);
								EEPROM_OP((u8*)&LockStatus,LockStatus_ADDR,1,MODE_W);
								//Bakkeybuf[0]=NEW_LOCK_GPRS_EVENT;
								//GBRec_Center_FIFO(Bakkeybuf,1,MODE_W);
							}
							else
							{
								OPResult=bNOT_OK|bOUT;
							}
							break;
					}
				}

				if(OPResult !=0x00)
				{
					tmp[index] = 0x01;index++;							//操作结果  下一个字节是具体原因
					if(OPResult == (bNOT_OK|bOUT))
						tmp[index] = 0x01;		//未插入锁杆
					if(OPResult == (bNOT_OK|bAGAIN))
						tmp[index] = 0x02;	//重复施封
					if(OPResult==(bNOT_OK|bCKBJ))
						tmp[index] = 0x03;		//有报警
					if(OPResult==(bNOT_OK|0x40))
						tmp[index] = 0x03;		//有报警
					if(OPResult==(bNOT_OK|0x80))
						tmp[index] = 0x03;		//有报警
					if(OPResult == (bNOT_OK|bVddLOW))
						tmp[index] = 0x04;		//低电压
					index++;
				}
				else
				{
					tmp[index] = 0x00;index++;
					tmp[index] = 0x00;index++;
				}
				memREset(tmp+index,0,10);			//扩展10字节
				index+=10;
				break;

			case DEVICE_UNLOCK:
				cmdback = DEVICE_UNLOCK_BACK;
				switch (LockStatus)
				{
					case LOCKWAIT:
						OPResult=bNOT_OK|bNOTSEAL;
						ReUnLock();
						break;

					case SEAL:
					case EXALARM:
						EEPROM_OP(keybuf1,SecKey6_ADDR,6,MODE_R);
						EEPROM_OP(keybuf2,SecKey6_ADDR2,6,MODE_R);
						if(buf[0] ==0x00)  //验证静态密码
						{
							if(!memCpare(buf+1,keybuf1,6))	//密码验证不成功
							{
								if(!memCpare(buf+1,keybuf2,6))
								{
										OPResult = bNOT_OK|bKEYERR;
										Bakkeybuf[0]=NEW_KEYERR_EVENT;
										GBRec_Center_FIFO(Bakkeybuf,1,MODE_W);
								}
							}
						}
						if((buf[0] == 0x01)||((buf[0] ==0x00)&&((memCpare(buf+1,keybuf1,6))||(memCpare(buf+1,keybuf2,6)))))
						{
							MotoDriver(bUnLOCK);
							OPResult=0x00;
							LockStatus=UNSEAL;
							EEPROM_OP((u8*)&LockStatus,LockStatus_ADDR,1,MODE_W);

							Bakkeybuf[0]=NEW_UNLOCK_GPRS_EVENT;
							GBRec_Center_FIFO(Bakkeybuf,1,MODE_W);
							AlarmCnt=0;
							AlarmType=0;
							EEPROM_OP((u8*)&AlarmType,AlarmType_ADDR,1,MODE_W);
							SeqCN++;
							EEPROM_OP((u8*)&SeqCN,SeqCN_ADDR,4,MODE_W);
						}
						break;
					case ALARM:
						OPResult=bNOT_OK|bALARM;
						break;

					case UNSEAL:
						OPResult=bNOT_OK|bAGAIN;
						ReUnLock();
						break;
					default:
						break;
				}
				if(OPResult ==0x00)
				{
					tmp[index] = 0x00;index++;//结果
					tmp[index] = 0x00;index++;//原因
				}
				else if(OPResult ==(bNOT_OK|bKEYERR))//密码错误
				{
					tmp[index] = 0x01;index++;//结果
					tmp[index] = 0x01;index++;//原因
				}
				else if(OPResult==(bNOT_OK|bNOTSEAL))//重复解封
				{
					tmp[index] = 0x01;index++;//结果
					tmp[index] = 0x02;index++;//原因
				}
				else if(OPResult==(bNOT_OK|bAGAIN))//重复解封
				{
					tmp[index] = 0x01;index++;//结果
					tmp[index] = 0x02;index++;//原因
				}
				else if(OPResult == (bNOT_OK|bALARM))//有报警  禁止开锁
				{
					tmp[index] = 0x01;index++;//结果
					tmp[index] = 0x03;index++;//原因
				}
				memREset(tmp+index,0,10);
				index+=10;
				break;

			case DEVICE_CLRALARM:
				cmdback = DEVICE_CLRALARM_BACK;

				switch (LockStatus)
				{
					case ALARM:
						LockStatus=EXALARM;
						EEPROM_OP((u8*)&LockStatus,LockStatus_ADDR,1,MODE_W);
						Bakkeybuf[0]=NEW_CLRALARM_EVENT;
						GBRec_Center_FIFO(Bakkeybuf,1,MODE_W);
						OPResult=0x00;
						break;
					case EXALARM:
						OPResult=bNO_ALARM;
						break;
					default:
						OPResult=bNO_ALARM;
						break;
				}
				if(OPResult ==0x00)
				{
					tmp[index] = 0x00;index++;
					tmp[index] = 0x00;index++;
				}
				else
				{
					tmp[index] = 0x01;index++;
					tmp[index] = 0x01;index++;
				}
				memREset(tmp+index,0,10);
				index+=10;
				break;
			default:
				break;
		}
		NewPacket_KY(tmp,index,cmdback,TCPLINK);
	}
	else if(op == KEY_OPR)
	{
		switch(cmd)
		{
			case DEVICE_LOCK:          //长按键关锁
				SendData2TestCOM((u8 *)&LockStatus,1,0);
				cmdback = DEVICE_KEY_LOCK_BACK;
				if(Vdd<0x32)//低于3.5v
				{
					OPResult=(bNOT_OK|bVddLOW);
				}
				else
				{
					switch(LockStatus)
					{
						case SEAL:				//0X40
						case EXALARM:			//0X90
							AlarmType=0;
							EEPROM_OP((u8*)&AlarmType,AlarmType_ADDR,1,MODE_W);
							OPResult=bNOT_OK|bAGAIN;
							MotoDriver(bLOCK);
							break;
						case ALARM:			//0X70
							if((AlarmType&OPENALARM)==OPENALARM)
							{ OPResult=bNOT_OK|bOUT;}//锁杆断开OPENALARM//RFTxBuf[dindex-1]=1;
							else  if((AlarmType&CUTALARM)==CUTALARM)
							{ OPResult=bNOT_OK|bCKBJ;}//非法拆开RFTxBuf[dindex-1]=1;
							else if((AlarmType&CKALARM)==CKALARM)
							{ OPResult=bNOT_OK|0x40;}
							else  if((AlarmType&YJKSALARM)==YJKSALARM)
							{ OPResult=bNOT_OK|0x80;}
							break;

						default:
							if(HALL_Check_B == 0)  //都闭合
							{
								MotoDriver(bLOCK);
								LockStatus=SEAL;
								OPResult=0x00;
								AlarmType=0;
								EEPROM_OP((u8*)&AlarmType,AlarmType_ADDR,1,MODE_W);
								SeqCN++;
								EEPROM_OP((u8*)&SeqCN,SeqCN_ADDR,4,MODE_W);
								SetDynamicPassword(DevParameters.snid,SeqCN);
//								printf("/r/nP:%d%d%d%d%d%d/r/n",SecKeyBuf6[0],SecKeyBuf6[1],SecKeyBuf6[2],SecKeyBuf6[3],SecKeyBuf6[4],SecKeyBuf6[5]);
								EEPROM_OP(SecKeyBuf6,SecKey6_ADDR,6,MODE_W);
								SysTick_Delay_Ms(2);
								EEPROM_OP(SecKeyBuf6,SecKey6_ADDR2,6,MODE_W);
								SysTick_Delay_Ms(2);
								EEPROM_OP((u8*)&LockStatus,LockStatus_ADDR,1,MODE_W);
								//Bakkeybuf[0]=NEW_LOCK_GPRS_EVENT;
								//GBRec_Center_FIFO(Bakkeybuf,1,MODE_W);
								keyopresult=4;
							}
							else
							{
								OPResult=bNOT_OK|bOUT;
							}
							break;
					}
				}

				if(OPResult == 0x00)
				{
					tmp[index] = 0x00;index++;
					tmp[index] = (u8)((SeqCN>>24)&0xff);index++;
					tmp[index] = (u8)((SeqCN>>16)&0xff);index++;
					tmp[index] = (u8)((SeqCN>>8)&0xff);index++;
					tmp[index] = (u8)((SeqCN)&0xff);index++;
					memREset(tmp+index,0,10);			//扩展10字节
					index+=10;

					NewPacket_KY(tmp,index,cmdback,TCPLINK);
				}
				break;

			case DEVICE_UNLOCK:
				cmdback = DEVICE_RF_KEY_UNLOCK_BACK;
				EEPROM_OP(keybuf1,SecKey6_ADDR,6,MODE_R);
				EEPROM_OP(keybuf2,SecKey6_ADDR2,6,MODE_R);
				switch (LockStatus)
				{
					case LOCKWAIT:
						if( memCpare(buf,keybuf1,6) || memCpare(buf,keybuf2,6))
						{
							OPResult=bNOT_OK|bNOTSEAL;
							ReUnLock();
							LockStatus=UNSEAL;
						}
						else
							OPResult = bNOT_OK|bKEYERR;
						break;
					case SEAL:
					case EXALARM:
						//if(buf[0] ==0x00)  //验证静态密码
						{
							if(!memCpare(buf,keybuf1,6))	//密码验证不成功
							{
								if(!memCpare(buf,keybuf2,6))
								{
										OPResult = bNOT_OK|bKEYERR;
										Bakkeybuf[0]=NEW_KEYERR_EVENT;
										GBRec_Center_FIFO(Bakkeybuf,1,MODE_W);
								}
							}
						}
						if( memCpare(buf,keybuf1,6) || memCpare(buf,keybuf2,6))
						{
							MotoDriver(bUnLOCK);
							OPResult=0x00;
							LockStatus=UNSEAL;
							EEPROM_OP((u8*)&LockStatus,LockStatus_ADDR,1,MODE_W);

							Bakkeybuf[0]=NEW_UNLOCK_KEY_EVENT;
							GBRec_Center_FIFO(Bakkeybuf,1,MODE_W);
							AlarmCnt=0;
							AlarmType=0;
							EEPROM_OP((u8*)&AlarmType,AlarmType_ADDR,1,MODE_W);
							SeqCN++;
							EEPROM_OP((u8*)&SeqCN,SeqCN_ADDR,4,MODE_W);
						}
						break;
					case ALARM:
						OPResult=bNOT_OK|bALARM;
						break;

					case UNSEAL:
						if( memCpare(buf,keybuf1,6) || memCpare(buf,keybuf2,6))
						{
							OPResult=bNOT_OK|bAGAIN;
							ReUnLock();
						}
						else
							OPResult = bNOT_OK|bKEYERR;
						break;
					default:
						break;
				}
				if(OPResult ==0x00)
				{
					keyopresult=0;
					tmp[index] = 0x00;index++;//结果
					tmp[index] = 0x00;index++;//原因
				}
				else if(OPResult ==(bNOT_OK|bKEYERR))//密码错误
				{
					tmp[index] = 0x01;index++;//结果
					tmp[index] = 0x01;index++;//原因
					keyopresult=1;
				}
				else if(OPResult==(bNOT_OK|bNOTSEAL))//重复解封
				{
					tmp[index] = 0x01;index++;//结果
					tmp[index] = 0x02;index++;//原因
					keyopresult=2;
				}
				else if(OPResult==(bNOT_OK|bAGAIN))//重复解封
				{
					tmp[index] = 0x01;index++;//结果
					tmp[index] = 0x02;index++;//原因
					keyopresult=2;
				}
				else if(OPResult == (bNOT_OK|bALARM))//有报警  禁止开锁
				{
					tmp[index] = 0x01;index++;//结果
					tmp[index] = 0x03;index++;//原因
					keyopresult=3;
				}
				tmp[index] = KEY_OPR;index++;//按键操作 标志位置1
				memREset(tmp+index,0,10);
				index+=10;
				NewPacket_KY(tmp,index,cmdback,TCPLINK);
				break;
			default:
				break;
		}
	}
}
/*******************************************************************************
* Function Name  : GB_RFACK
* Description    : GB_RFACK
* Input          : None.
* Return         : None.
*******************************************************************************/
void GB_RFACK(void)
{
	u8 dindex,i;
	u16 crc;

	RFM_Ini((u8*)LockCHcfg);

	//协议ID	锁ID	应答内容长度	应答数据	校验码
	dindex=0;
	RFTxBuf[dindex]=XYID;dindex++;
	memcopy(RFTxBuf+dindex,GB_LOCKIDbuf,8);dindex+=8;//SRC ID
	RFTxBuf[dindex]=0;dindex++;//LEN

	crc=MakeCRC16(RFTxBuf,dindex);
	RFTxBuf[dindex]=(u8)(crc>>8);dindex++;
	RFTxBuf[dindex]=(u8)(crc);dindex++;

	for(i=dindex;i<30;i++)
	{
	RFTxBuf[i]=0x30;
	}
	GB_LockSend(RFTxBuf,30);

	SysTick_Delay_Ms(5);//KKKK
	RFM_Ini((u8*)LockCHcfg);
}

/*******************************************************************************
* Function Name  : GB_RFTxBufGet
* Description    : GB_RFTxBufGet
* Input          : None.
* Return         : None.
*******************************************************************************/

u8 GB_RFTxBufGet(u8 rfcmd)
{
	u8 fscd,dindex,gprsindex;
	u16 crc;

	union
	{
		u32 difftime32;
		u8  difftime8[4];
	}diftt;

	dindex=0;
	gprsindex = 1;
	//Send2CCBuf[0]=1;
	RFTxBuf[dindex]=XYID;
	dindex++;
	RFTxBuf[dindex]=0x80;
	if(GB_LOCK_INFO_UP!=rfcmd)
	{
		RFTxBuf[dindex]|=bP2P;
	}
	dindex++;

	memcopy(RFTxBuf+dindex,RFRxBuf+10,8);dindex+=8;//DEST ID
	memcopy(RFTxBuf+dindex,GB_LOCKIDbuf,8);dindex+=8;//SRC ID
	RFTxBuf[dindex]=rfcmd;dindex++;//18 rfcmd
	RFTxBuf[dindex]=0;dindex++;//19 参数长度

	switch (rfcmd)
	{
		case GB_LOCK_BACK:
#if(NEED_EW)
		case PRELOCK_BACK://EW//
#endif
			RFTxBuf[dindex]=OPResult;dindex++;
			RFTxBuf[dindex]=GBVdd;dindex++;
			Send2CCBuf[gprsindex] = ((DEVICE_RF_LOCK_BACK&0xff00)>>8);gprsindex++;
			Send2CCBuf[gprsindex] = (DEVICE_RF_LOCK_BACK&0xff);gprsindex++;
			if(OPResult==0)
			{
				Send2CCBuf[gprsindex] = 0x00;gprsindex++;
				Send2CCBuf[gprsindex] = 0x00;gprsindex++;
			}
			else if(OPResult==(bNOT_OK|bOUT))
			{
				Send2CCBuf[gprsindex] = 0x01;gprsindex++;
				Send2CCBuf[gprsindex] = 0x01;gprsindex++;
			}
			else if(OPResult==(bNOT_OK|bAGAIN))
			{
				Send2CCBuf[gprsindex] = 0x01;gprsindex++;
				Send2CCBuf[gprsindex] = 0x02;gprsindex++;
			}
			else if(OPResult==(bNOT_OK|bCKBJ))
			{
				Send2CCBuf[gprsindex] = 0x01;gprsindex++;
				Send2CCBuf[gprsindex] = 0x03;gprsindex++;
			}
			else if(OPResult==(bNOT_OK|0x40))
			{
				Send2CCBuf[gprsindex] = 0x01;gprsindex++;
				Send2CCBuf[gprsindex] = 0x03;gprsindex++;
			}
			else if(OPResult==(bNOT_OK|0x80))
			{
				Send2CCBuf[gprsindex] = 0x01;gprsindex++;
				Send2CCBuf[gprsindex] = 0x03;gprsindex++;
			}
			else if(OPResult == (bNOT_OK|bVddLOW))
			{
				Send2CCBuf[gprsindex] = 0x01;gprsindex++;
				Send2CCBuf[gprsindex] = 0x04;gprsindex++;
			}
			Send2CCBuf[gprsindex] = OFFLINE_OPR;gprsindex++;
			memcopy(Send2CCBuf+gprsindex,RFRxBuf+10,8);
			gprsindex+=8;
			memREset(Send2CCBuf+gprsindex,0,2);
			gprsindex+=2;
			Send2CCBuf[0] = gprsindex;

			if(OPResult==0)
			{
				AlarmCnt=0;
				RFTxBuf[dindex]=0x01;//成功//
				if(NEWcmd)
				{
					SeqCN++;
					NEWcmd=0;
					EEPROM_OP((u8*)&SeqCN,SeqCN_ADDR,4,MODE_W);
				}
			}
			else
			{
				RFTxBuf[dindex]=0XFF;//不成功//
			}
			dindex++;
			RTC_OP(RFTxBuf+dindex,RTC_GETTIME_GB);
			dindex+=8;
			diftt.difftime32=(SeqCN);//计数器//
			RFTxBuf[dindex]=diftt.difftime8[3];dindex++;
			RFTxBuf[dindex]=diftt.difftime8[2];dindex++;
			RFTxBuf[dindex]=diftt.difftime8[1];dindex++;
			RFTxBuf[dindex]=diftt.difftime8[0];dindex++;
			fscd=dindex;
			break;

		case GB_UNLOCK_BACK:
		case GB_CHECK_BACK:
			RFTxBuf[dindex]=OPResult;dindex++;
			RFTxBuf[dindex]=GBVdd;dindex++;
			if(rfcmd == GB_UNLOCK_BACK)
			{
				Send2CCBuf[gprsindex] = ((DEVICE_RF_KEY_UNLOCK_BACK&0xff00)>>8);gprsindex++;
				Send2CCBuf[gprsindex] = (DEVICE_RF_KEY_UNLOCK_BACK&0xff);gprsindex++;
				if(OPResult==0)
				{
					Send2CCBuf[gprsindex] = 0x00;gprsindex++;
					Send2CCBuf[gprsindex] = 0x00;gprsindex++;
				}
				else if(OPResult==(bNOT_OK|bKEYERR))
				{
					Send2CCBuf[gprsindex] = 0x01;gprsindex++;
					Send2CCBuf[gprsindex] = 0x01;gprsindex++;
				}
				else if(OPResult==(bNOT_OK|bNOTSEAL))//重复解封
				{
					Send2CCBuf[gprsindex] = 0x01;gprsindex++;
					Send2CCBuf[gprsindex] = 0x02;gprsindex++;

				}
				else if(OPResult==(bNOT_OK|bAGAIN))//重复解封
				{
					Send2CCBuf[gprsindex] = 0x01;gprsindex++;
					Send2CCBuf[gprsindex] = 0x02;gprsindex++;

				}
				else if(OPResult == (bNOT_OK|bALARM))//有报警  禁止开锁
				{
					Send2CCBuf[gprsindex] = 0x01;gprsindex++;
					Send2CCBuf[gprsindex] = 0x03;gprsindex++;
				}
				Send2CCBuf[gprsindex] = OFFLINE_OPR;gprsindex++;
				memcopy(Send2CCBuf+gprsindex,RFRxBuf+10,8);
				gprsindex+=8;
				memREset(Send2CCBuf+gprsindex,0,2);
				gprsindex+=2;
				Send2CCBuf[0] = gprsindex;
			}

			if(GB_UNLOCK_BACK==rfcmd)
			{
				if(OPResult==0)
				{
					RFTxBuf[dindex]=0x01;//成功//
					if(NEWcmd)
					{
						SeqCN++;
						EEPROM_OP((u8*)&SeqCN,SeqCN_ADDR,4,MODE_W);
					}
				}
				else
				{
					RFTxBuf[dindex]=0XFF;//不成功//
				}
			}
			else
			{
				if(OPResult==0X05)
				{
					RFTxBuf[dindex]=0xff;//不成功//
				}
				else
				{
					RFTxBuf[dindex]=0x01;//成功//
				}
			}
			dindex++;
			RTC_OP(RFTxBuf+dindex,RTC_GETTIME_GB);
			dindex+=8;
			RFTxBuf[dindex]=AlarmCnt;dindex++;  //报警次数AlarmCnt//
			//当前报警种类//
			RFTxBuf[dindex]=0;
			if((AlarmType&OPENALARM)==OPENALARM)
			{
				RFTxBuf[dindex]|=0x01;//锁杆断开OPENALARM//RFTxBuf[dindex-1]=1;
			}
			else if((AlarmType&CUTALARM)==CUTALARM)
			{
				RFTxBuf[dindex]|=0x02;//非法拆开RFTxBuf[dindex-1]=1;
			}
			else  if((AlarmType&CKALARM)==CKALARM){ RFTxBuf[dindex]|=0x03;}
			else  if((AlarmType&YJKSALARM)==YJKSALARM){ RFTxBuf[dindex]|=0x04;}
			dindex++;
			//当前报警时间//
			memcopy(RFTxBuf+dindex,AlarmTimebuf,8);dindex+=8;
			if(rfcmd==GB_UNLOCK_BACK)
			{
				diftt.difftime32=(SeqCN);//计数器//
				//memcopy(RFTxBuf+dindex,(u8*)diftt.difftime8,4);dindex+=4;
				RFTxBuf[dindex]=diftt.difftime8[3];dindex++;
				RFTxBuf[dindex]=diftt.difftime8[2];dindex++;
				RFTxBuf[dindex]=diftt.difftime8[1];dindex++;
				RFTxBuf[dindex]=diftt.difftime8[0];dindex++;
			}
			else
			{
				RFTxBuf[dindex]=0;dindex++;//扩展数据包总数 补0//
			}
			fscd=dindex;
			break;

		case GB_CLR_ALARM_BACK:
			RFTxBuf[dindex]=OPResult;dindex++;
			RFTxBuf[dindex]=GBVdd;dindex++;
			
			RTC_OP(RFTxBuf+dindex,RTC_GETTIME_GB);
			dindex+=8;
			fscd=dindex;
			Send2CCBuf[gprsindex] = ((DEVICE_RF_CLRALARM_BACK&0xff00)>>8);gprsindex++;
			Send2CCBuf[gprsindex] = (DEVICE_RF_CLRALARM_BACK&0xff);gprsindex++;
			if(OPResult ==0x00)
			{
				Send2CCBuf[gprsindex] = 0x00;gprsindex++;
				Send2CCBuf[gprsindex] = 0x00;gprsindex++;
			}
			else
			{
				Send2CCBuf[gprsindex] = 0x01;gprsindex++;
				Send2CCBuf[gprsindex] = 0x01;gprsindex++;
			}
			Send2CCBuf[gprsindex] = OFFLINE_OPR;gprsindex++;
			memcopy(Send2CCBuf+gprsindex,RFRxBuf+10,8);
			gprsindex+=8;
			memREset(Send2CCBuf+gprsindex,0,2);
			gprsindex+=2;
			Send2CCBuf[0] = gprsindex;
			break;
		case GB_YJCLR_ALARM_BACK:
			Send2CCBuf[0]=0;
			RFTxBuf[dindex]=OPResult;dindex++;
			RFTxBuf[dindex]=GBVdd;dindex++;
			RTC_OP(RFTxBuf+dindex,RTC_GETTIME_GB);
			dindex+=8;
			fscd=dindex;
			break;
		case GB_RD_EVENTDATA_BACK:
			RFTxBuf[20]=OPResult;dindex++;//okflag//
			RFTxBuf[21]=RecSeq;dindex++;//bcn//
			RFTxBuf[22]=TotalRecNum1;dindex++;//

			if(OPResult==0x01)
			{
				crc=0;
				if((RecSeq >= 1)&&(RecSeq <= TotalRecNum1))
				{
					//crc=EEPROM_OP(RFTxBuf+23,LockEVENT1_ADDR+(RecSeq-1)*48,RECORDDATALEN,MODE_R);
					//crc=EEPROM_OP(RFTxBuf+23,LockEVENT1_ADDR+(TotalRecNum1 - RecSeq)*RECORDDATALEN,RECORDDATALEN,MODE_R);
					if(TotalRecNum1 < MAXBC_NUM)
					  crc = EEPROM_OP(RFTxBuf+23,((u32)(LockEVENT1_ADDR)+(u32)(RecSeq - 1)*RECORDDATALEN),RECORDDATALEN,MODE_R);
					else
					  crc = EEPROM_OP(RFTxBuf+23,((u32)(LockEVENT1_ADDR)+((u32)(RecordIndex + RecSeq -1)% MAXBC_NUM)*RECORDDATALEN),RECORDDATALEN,MODE_R);
					RFTxBuf[23] = RecSeq;

					if(crc==0)
					{
						RFTxBuf[20]=0x02;//READ_ERROR//
						memREset(RFTxBuf+23,0xff,RECORDDATALEN);
					}
				}
				if(RecSeq > TotalRecNum1)
				{
					RFTxBuf[20]=0x03;//无事件数据//
					memREset(RFTxBuf+23,0xff,RECORDDATALEN);
				}
			}
			else if(OPResult==0x04)
				memREset(RFTxBuf+23,0xff,RECORDDATALEN);
			dindex += RECORDDATALEN;
			RFTxBuf[dindex] = RFRxBuf[31];     //通道号...
			dindex++;
			fscd = dindex;
			break;

		case GB_RD_CID_BACK:
		case GB_WR_CID_BACK:
			break;

		case GB_READ_LOCK_PKEY_BACK:
			break;

		case GB_DL_READER_PKEY_BACK://下传阅读器公钥，计算密钥成功后回复//
			break;

		case 0xE4:
			break;

		case RZ_READER_BACK:
			break;

		case MADE_AQZS_BACK:
			break;

		case RESET_SAFEIC_BACK:
		case SET_GATE_BACK:
		case SET_RFADD_BACK:
		case CERTIFI_INIT_BACK:
		case SET_GPS_MODE_BACK:

		case SET_SLEEP_TIME_BACK:

			Send2CCBuf[0]=0;
			RFTxBuf[dindex]=OPResult;
			dindex++;
			fscd=dindex;
			break;

		case GB_LOCK_INFO_UP:
		//锁电压	锁状态	报警种类	安全智能锁公钥	保留
			Send2CCBuf[0]=0;
			srand((u16)LockTicker);
			RFsnBack=rand();
			RFTxBuf[dindex]=GBVdd;dindex++;
			RFTxBuf[dindex]=0;
			if(ACC_Off_flag=='1')
			{
				RFTxBuf[dindex]=0x01;//拔出状态//
			}
			switch(LockStatus)
			{
				case SEAL:
				case ALARM:
				case EXALARM:
					break;
				default:
					RFTxBuf[dindex]|=0x02;//解封状态//
					break;
			}
			dindex++;

			//当前报警种类2//
			RFTxBuf[dindex]=0;
			if(Vdd<0x32){RFTxBuf[dindex]|=0x01;}
			if((AlarmType&OPENALARM)==OPENALARM){ RFTxBuf[dindex]|=0x02;}//锁杆断开OPENALARM//RFTxBuf[dindex-1]=1;
			else if((AlarmType&CUTALARM)==CUTALARM){ RFTxBuf[dindex]|=0x04;}//非法拆开RFTxBuf[dindex-1]=1;
			else  if((AlarmType&CKALARM)==CKALARM){ RFTxBuf[dindex]|=0x08;}
			else  if((AlarmType&YJKSALARM)==YJKSALARM){ RFTxBuf[dindex]|=0x10;}
			dindex++;
			memREset(RFTxBuf+dindex,0x00,64);dindex+=64;

			RFTxBuf[dindex]=0;
			dindex++;

			fscd=dindex;

			break;

#if(NEED_EW)
		case GB_GPSGSM_OFF_BACK://EW//
		case GB_GPSGSM_ON_BACK://EW//
		case GB_GPSOFF_GSMON_BACK://EW//
		case GB_NeedCheckKEY_YES_BACK:
		case GB_NeedCheckKEY_NO_BACK:
		case GB_NeedLoWPower_BACK:
		case GB_NeedGprsOperate_BACK:
		case GB_SetSIMNumber_BACK:
			Send2CCBuf[0]=0;
			RFTxBuf[dindex]=OPResult;dindex++;
			fscd=dindex;
			break;

		case GB_WR_CHECKFLAG_BACK://EW//
		case GB_RD_CHECKFLAG_BACK://EW//
			Send2CCBuf[0]=0;
			RFTxBuf[dindex]=OPResult;dindex++;
			RFTxBuf[dindex]=0;dindex++;
			//if(CheckFlag==1){CheckFlag=0;}
			fscd=dindex;
			break;
#endif

		default:
			fscd=20;
			break;
	}
	RFTxBuf[19]=fscd-20;//明文参数长度
////////////////////////////////////////////////

#if(NEED_SENDRFR2CC)
	if(Send2CCBuf[0])
	{
		memcopy(Send2CCBuf+1,RFTxBuf,dindex);
		Send2CCBuf[0]=dindex;
	}
#else
#endif

	crc=MakeCRC16(RFTxBuf,dindex);
	RFTxBuf[dindex]=(u8)((crc>>8)&0x00ff);dindex++;
	RFTxBuf[dindex]=(u8)crc;dindex++;
	return (dindex);
}

/*******************************************************************************
* Function Name  : GB_RFTxDatPacketSend
* Description    : GB_RFTxDatPacketSend
* Input          : None.
* Return         : None.
*******************************************************************************/
void GB_RFTxDatPacketSend(u8 cmd,u8 mode,u8 trycnt)
{
	u8 first;

	RFM_Ini((u8*)LockCHcfg);
	first=mode;
	if(first==1)
	{
		RFSendLen=GB_RFTxBufGet(cmd);
	}

	GB_LockSend(RFTxBuf,RFSendLen);

	RFM_Ini((u8*)LockCHcfg);
}
/*******************************************************************************
* Function Name  : NEW_RFTxBufGet
* Description    : NEW_RFTxBufGet
* Input          : None.
* Return         : None.
*******************************************************************************/
u8 NEW_RFTxBufGet(u8 rfcmd)
{
	u8 dindex,gprsindex;
	u8 crcxor;


	dindex=0;
	gprsindex = 1;
	RFTxBuf[dindex]=HEAD;
	dindex++;
	RFTxBuf[dindex]=HEAD;
	dindex++;

	RFTxBuf[2]=0x00;   //len
	dindex++;
	RFTxBuf[3]=0x00;
	dindex++;

	RFTxBuf[dindex]=rfcmd;    //cmd
	dindex++;

	memcopy(RFTxBuf+dindex,RFRxBuf+5,8);dindex+=6;//snid

	switch (rfcmd)
	{
		case NEWRF_LOCK_BACK:
			RFTxBuf[dindex]=RFRxBuf[11];dindex++;
			memcopy(RFTxBuf+dindex,RFRxBuf+12,6);
			dindex+=6;
			Send2CCBuf[gprsindex] = ((DEVICE_RF_LOCK_BACK&0xff00)>>8);gprsindex++;
			Send2CCBuf[gprsindex] = (DEVICE_RF_LOCK_BACK&0xff);gprsindex++;

			if(OPResult==0)
			{
				AlarmCnt=0;
				RFTxBuf[dindex]=0x00;//成功//
				dindex++;
				RFTxBuf[dindex]=0x00;
				dindex++;
				Send2CCBuf[gprsindex] = 0x00;gprsindex++;
				Send2CCBuf[gprsindex] = 0x00;gprsindex++;
			}
			else if(OPResult==(bNOT_OK|bOUT))
			{
				RFTxBuf[dindex]=0X01;//未插入锁杆
				dindex++;
				RFTxBuf[dindex]=0x01;
				dindex++;
				Send2CCBuf[gprsindex] = 0x01;gprsindex++;
				Send2CCBuf[gprsindex] = 0x01;gprsindex++;
			}
			else if(OPResult==(bNOT_OK|bAGAIN))
			{
				RFTxBuf[dindex]=0X01;//重复施封
				dindex++;
				RFTxBuf[dindex]=0x02;
				dindex++;
				Send2CCBuf[gprsindex] = 0x01;gprsindex++;
				Send2CCBuf[gprsindex] = 0x02;gprsindex++;

			}
			else if(OPResult==(bNOT_OK|bCKBJ))
			{
				RFTxBuf[dindex]=0X01;//有报警//
				dindex++;
				RFTxBuf[dindex]=0x03;
				dindex++;

				Send2CCBuf[gprsindex] = 0x01;gprsindex++;
				Send2CCBuf[gprsindex] = 0x03;gprsindex++;
			}
			else if(OPResult==(bNOT_OK|0x40))
			{
				RFTxBuf[dindex]=0X01;//有报警//
				dindex++;
				RFTxBuf[dindex]=0x03;
				dindex++;
				Send2CCBuf[gprsindex] = 0x01;gprsindex++;
				Send2CCBuf[gprsindex] = 0x03;gprsindex++;

			}
			else if(OPResult==(bNOT_OK|0x80))
			{
				RFTxBuf[dindex]=0X01;//有报警//
				dindex++;
				RFTxBuf[dindex]=0x03;
				dindex++;
				Send2CCBuf[gprsindex] = 0x01;gprsindex++;
				Send2CCBuf[gprsindex] = 0x03;gprsindex++;
			}
			else if(OPResult == (bNOT_OK|bVddLOW))
			{
				RFTxBuf[dindex]=0X01;//低电压//
				dindex++;
				RFTxBuf[dindex]=0x04;
				dindex++;
				Send2CCBuf[gprsindex] = 0x01;gprsindex++;
				Send2CCBuf[gprsindex] = 0x04;gprsindex++;
			}
			memREset(RFTxBuf+dindex,0,10);
			dindex+=10;
			Send2CCBuf[gprsindex] = OFFLINE_OPR;gprsindex++;
			memREset(Send2CCBuf+gprsindex,0,10);
			gprsindex+=10;
			Send2CCBuf[0] = gprsindex;
			break;

		case NEWRF_UNLOCK_BACK:
			RFTxBuf[dindex]=RFRxBuf[11];dindex++;

			Send2CCBuf[gprsindex] = ((DEVICE_RF_KEY_UNLOCK_BACK&0xff00)>>8);gprsindex++;
			Send2CCBuf[gprsindex] = (DEVICE_RF_KEY_UNLOCK_BACK&0xff);gprsindex++;
			if(OPResult==0)
			{
				AlarmCnt=0;
				RFTxBuf[dindex]=0x00;//成功//
				dindex++;
				RFTxBuf[dindex]=0x00;
				dindex++;
				Send2CCBuf[gprsindex] = 0x00;gprsindex++;
				Send2CCBuf[gprsindex] = 0x00;gprsindex++;
			}
			else if(OPResult==(bNOT_OK|bKEYERR))
			{
				RFTxBuf[dindex]=0X01;//不成功//
				dindex++;
				RFTxBuf[dindex]=0x01;
				dindex++;
				Send2CCBuf[gprsindex] = 0x01;gprsindex++;
				Send2CCBuf[gprsindex] = 0x01;gprsindex++;
			}
			else if(OPResult==(bNOT_OK|bNOTSEAL))//重复解封
			{
				RFTxBuf[dindex]=0X01;//不成功//
				dindex++;
				RFTxBuf[dindex]=0x02;
				dindex++;
				Send2CCBuf[gprsindex] = 0x01;gprsindex++;
				Send2CCBuf[gprsindex] = 0x02;gprsindex++;

			}
			else if(OPResult==(bNOT_OK|bAGAIN))//重复解封
			{
				RFTxBuf[dindex]=0X01;//不成功//
				dindex++;
				RFTxBuf[dindex]=0x02;
				dindex++;
				Send2CCBuf[gprsindex] = 0x01;gprsindex++;
				Send2CCBuf[gprsindex] = 0x02;gprsindex++;

			}
			else if(OPResult == (bNOT_OK|bALARM))//有报警  禁止开锁
			{
				RFTxBuf[dindex]=0X01;//不成功//
				dindex++;
				RFTxBuf[dindex]=0x03;
				dindex++;
				Send2CCBuf[gprsindex] = 0x01;gprsindex++;
				Send2CCBuf[gprsindex] = 0x03;gprsindex++;

			}
			memREset(RFTxBuf+dindex,0,10);
			dindex+=10;
			Send2CCBuf[gprsindex] = OFFLINE_OPR;gprsindex++;
			memREset(Send2CCBuf+gprsindex,0,10);
			gprsindex+=10;
			Send2CCBuf[0] = gprsindex;

			break;
		case NEWRF_CHECK_BACK:
			if(OPResult == 0x02)
			{
				RFTxBuf[dindex]=0x00;dindex++;
			}
			else
			{
				RFTxBuf[dindex]=0x01;dindex++;
			}
			RFTxBuf[dindex]=(Adc_Vdd>>8);dindex++;
			RFTxBuf[dindex]=(Adc_Vdd&0xff);dindex++;

			memREset(RFTxBuf+dindex,0,10);
			dindex+=10;
			break;

		case NEWRF_CLR_ALARM_BACK:
			Send2CCBuf[gprsindex] = ((DEVICE_RF_CLRALARM_BACK&0xff00)>>8);gprsindex++;
			Send2CCBuf[gprsindex] = (DEVICE_RF_CLRALARM_BACK&0xff);gprsindex++;
			if(OPResult ==0x00)
			{
				RFTxBuf[dindex] = 0x00;dindex++;
				RFTxBuf[dindex] = 0x00;dindex++;
				Send2CCBuf[gprsindex] = 0x00;gprsindex++;
				Send2CCBuf[gprsindex] = 0x00;gprsindex++;

			}
			else
			{
				RFTxBuf[dindex] = 0x01;dindex++;
				RFTxBuf[dindex] = 0x01;dindex++;
				Send2CCBuf[gprsindex] = 0x01;gprsindex++;
				Send2CCBuf[gprsindex] = 0x01;gprsindex++;

			}
			memREset(RFTxBuf+dindex,0,10);
			dindex+=10;
			Send2CCBuf[gprsindex] = OFFLINE_OPR;gprsindex++;
			memREset(Send2CCBuf+gprsindex,0,10);
			gprsindex+=10;
			Send2CCBuf[0] = gprsindex;
			break;

		case NEWRF_RD_EVENTDATA_BACK:

			if(OPResult==0x01)
			{

				if((RecSeq >= 1)&&(RecSeq <= TotalRecNum1))
				{
					if(TotalRecNum1 < MAXBC_NUM)
					  EEPROM_OP(RFTxBuf+dindex,((u32)(LockNEWEVENT_ADDR)+(u32)(RecSeq - 1)*NEWRECORDDATALEN),NEWRECORDDATALEN,MODE_R);
					else
					  EEPROM_OP(RFTxBuf+dindex,((u32)(LockNEWEVENT_ADDR)+((u32)(RecordIndex + RecSeq -1)% MAXBC_NUM)*NEWRECORDDATALEN),NEWRECORDDATALEN,MODE_R);
  					dindex+=NEWRECORDDATALEN;
					RFTxBuf[11] = RecSeq;
				}
				if(RecSeq > TotalRecNum1)
				{
 					memREset(RFTxBuf+dindex,0xff,8);
					dindex+=8;
				}
			}
			RFTxBuf[dindex]=0x00;
			dindex++;
 			memREset(RFTxBuf+dindex,0,10);
			dindex+=10;
			break;
	}

	RFTxBuf[2]=((dindex+1)>>0x08);   //len
	RFTxBuf[3]=((dindex+1)&0xff);

	crcxor = Do_XOR(RFTxBuf,dindex);
	RFTxBuf[dindex] = crcxor;
	dindex++;
	return (dindex);
}

/*******************************************************************************
* Function Name  : NEW_RFTxDatPacketSend
* Description    : NEW_RFTxDatPacketSend
* Input          : None.
* Return         : None.
*******************************************************************************/
void NEW_RFTxDatPacketSend(u8 cmd,u8 mode,u8 trycnt)
{
	u8 first;

	RFM_Ini((u8*)LockCHcfg);
	first=mode;
	if(first==1)
	{
		RFSendLen = NEW_RFTxBufGet(cmd);
	}
	GB_LockSend(RFTxBuf,RFSendLen);
	RFM_Ini((u8*)LockCHcfg);
}

/*******************************************************************************
* Function Name  : KEY_VN
* Description    : KEY_VN
* Input          : None.
* Return         : None.
*******************************************************************************/

u8 KEY_VN(u8 flag,u8 rrcmd)
{
	u8 Bakkeybuf[11];

	if(!memCpare(RFRxBuf+20,SecKeyBuf,10))
	{
		EEPROM_OP(Bakkeybuf,SecKey_ADDR2,10,MODE_R);
		EEPROM_OP(SecKeyBuf,SecKey_ADDR,10,MODE_R);
		if(!memCpare(RFRxBuf+20,Bakkeybuf,10))
		{
#if(NEED_WNKEY)
			if(!memCpare(RFRxBuf+20,(u8*)WNMM,10))
#endif
			SysTick_Delay_Ms(1);

			{
				OPResult=flag;
				GB_RFTxDatPacketSend(rrcmd,1,1);
				Bakkeybuf[0]=GB_KEYERR_EVENT;
				GBRec_FIFO(Bakkeybuf,0,MODE_W);
				return 0;
			}
		}
	}
	return 1;
}


void SetDynamicPassword(u8* sn,u32 count)
{
	u8 i,temp[6];
	u8 tempnum;

	tempnum=(u8)((count>>24)+55) ^(u8) ((count>>16) +66)^(u8) ((count>>8)+77) ^ (u8)(count+88);

	for(i=0;i<6;i++)
	{
		temp[i] =(sn[i]+i*2+3) |(tempnum+i*5+1);
		temp[i] ^=tempnum-i;
		SecKeyBuf6[i] =temp[i]%10;
		SecKeyBuf6[i] =SecKeyBuf6[i]+0x30;
	}

//	SecKeyBuf6[0] =0x31;
//	SecKeyBuf6[1] =0x31;
//	SecKeyBuf6[2] =0x31;
//	SecKeyBuf6[3] =0x31;
//	SecKeyBuf6[4] =0x31;
//	SecKeyBuf6[5] =0x31;
}





