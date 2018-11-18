//本文档包含GPRS的通讯协议
//函数名称定义：GPRS_具体内容;
//函数名称定义：SMS_具体内容;
#include "gprs.h"
#include "rfid.h"
#include "common.h"
#include "TTLM.h"
#include "gbprotocol.h"
#include "crcencoder.h"
#include "gps.h"
#include "BSprotocol.h"
#include "global.h"
#include "lp_mode.h"

const u8 RFID_ULOP[]={0X03,0X2D};//cmd,ywsj

u8 TrIndex = 0;
u8 ConnectIndex=0;
u8 GpsrCregFlag = 0;
u8 gprsindex =0;
u8 gpsriniflag = 0;
u8 CSQValue = 99;
u8 qimuxflag =0;
u32 HandTick = 0;
u8	ModuleTempClosedCount = 0;
u8 ipaddr[15]= {'0','0','0','.','0','0','0','.','0','0','0','.','0','0','0'};;
u8   LastATSending;
u8	 UART1_TASK_STATUS;
u8	 UART1_TASK_ID;
u8	 ATResponse;
u8	 AT_Status;
u8	 ATRE_Null_Cnt;
u8	 ATRE_Err_Cnt;
u8   ATSend_Err_Cnt;
u8   ATSend_Err_Cnt11;
u16  ATRe_WaitTick;
u8   AT_Ready;
u8	 SIM_Error_Bit;
u16  SIM_Error_Cnt;
u8   SIMGeted;
u8	 GSMRegisted;
u16  GSMNORegistCnt;
u8	 GPRSRegisted;
u16  GPRSNORegistCnt;
u16  GSMCHECKcnt;
u16  TCPCHECKcnt;
u16  TCP_ErrorCnt;
u16  TCP_ErrorCnt1;
u8	 GPRSReConnectFlag;
u16  GPRS_NET_TIPtick;
u8	 MODULE_RESETed;
u16  MODULE_RESETed_Cnt;
u8 AT_CMD_list[AT_CMD_CNT][115];//15*115
u8 IP_RCV_Buff[IP_RECBUF_CNT][IPBUF_SIZE];//应用层 平台数据 开关锁心跳
u8 AT_RCV_Buff[AT_INBUF_CNT][AT_INBUF_SIZE];	//接收到的AT数据的缓存//10*80   AT  M26
u8 SMS_RCV_Buff[SMS_RECBUF_CNT][200];
u8  SMSDATAbuf[150];
u8 ipaddrlen;
u8 ReconnectTimes;
u8 SysTicker;
u8 SMS_SEND_BUF[150];
u8 cclk_flag=0;

/*******************************************************************************
* Function Name  : Get_Gprs_DTR
* Description    : Get_Gprs_DTR
* Input          :
* Return         :
*******************************************************************************/
u8 Get_Gprs_DTR(void)
{
	return (GPIO_ReadOutputDataBit(GPRS_DTR_PORT,GPRS_DTR_PIN));
}

/*******************************************************************************
* Function Name  : NetModuleBaseIni
* Description    : 初始化 时发的命令  at指令
* Input          :
* Return         :
*******************************************************************************/
void NetModuleBaseIni(void)
{
	ATCmdFIFO((u8 *)"ATE0\r\0",SET_ATE);
	ATCmdFIFO((u8 *)"AT+CPIN?\r\0",ASK_PIN);
	ATCmdFIFO((u8 *)"AT+CMEE=0;\r\0",SET_CMEE);
	ATCmdFIFO((u8 *)"AT+CREG=2\r\0",SET_CREG);
	GpsrCregFlag = 1;
	ATCmdFIFO((u8 *)"AT+QSCLK=1\r\0",SET_ATE);
	ATCmdFIFO((u8 *)"AT&W\r\0",SET_STORE);
}
/*******************************************************************************
* Function Name  : SMSIni
* Description    : SMSIni
* Input          :
* Return         :
*******************************************************************************/
void SMSIni(void)
{
	//1、 AT+CSCS=“GSM”//设置模块字符集，是采用GSM还是UCS2
	//2、 AT+CSDH=0 //设置文本显示格式 默认
	//3、 AT+CMGF=1 //设置模块短信发送格式：1TXT还是0PDU
	//4、 AT+CSMP=17,167,0,241//这个AT命令仅限在TXT模式下使用 默认
	//5、 AT+CNMI=2,2,0,0,0 //设置短消息接受方式.+CMT
	//6、 AT+CMGL=“ALL” //读取所有短信
	ATCmdFIFO((u8 *)"AT+CMGF=1\r\0",SMS_SET); //txt//
	ATCmdFIFO((u8 *)"AT+CNMI=2,2,0,0,0\r\0",SMS_TIP_SET);//+CMT
}
/*******************************************************************************
* Function Name  : GPRSIni
* Description    : GPRSIni
* Input          :
* Return         :
*******************************************************************************/
void GPRSIni(void)
{
	//AT+QIREGAPP?查询APN
	//1、AT+QIDEACT //关闭GPRS连接 2、ATH
	//3、AT+QINDRI=1 //打开模块的RI引脚功能 默认
	//4、AT+QICSGP=1,"CMNET","",""//设置模块GPRS的APN，用户名，密码
	//5、AT+QITCFG=3,2,550,1 //配置数据缓存透传模式AT+QIMUX=0下有效//
	//6、AT+QIMUX=0 //设置模块多路连接：1路连接
	//7、AT+QIMODE=0 //设置模块传输方式：非透传 默认
	//8、AT+QIDNSIP=0 //配置模块为IP连接 默认
	//9、AT+QIACT //激活模块的GPRS网络
	//10、AT+QILOCIP //查询模块IP
	//11、AT+QIOPEN="TCP","58.60.185.172","8886" //做第一路TCP连接

	//UARTWrite((u8*)"GPRSini\r\n",9,DEBUG_COM);//debuggggggggg
	//Delay5MS(1);
	//ATCmdFIFO((u8 *)"AT+QIDEACT\r\0",GPRS_DEACT); //IF NEED//

	//ATCmdFIFO((u8 *)"AT+QICSGP=1,\"CMNET\"\r\0",GPRS_APN);
	ATCmdFIFO((u8 *)APN_PARA,GPRS_APN);
	SysTick_Delay_Ms(10);
	ATCmdFIFO((u8 *)"AT+QIMUX=1\r\0",GPRS_SET);
	SysTick_Delay_Ms(10);
	//ATCmdFIFO((u8 *)"AT+QIMODE=0\r\0",GPRS_SET);
	//ATCmdFIFO((u8 *)"AT+QIHEAD=0\r\0",GPRS_SET);//=1 IPD<len>:
	ATCmdFIFO((u8 *)"AT+QIMUX?\r\0", GPRS_SET);
	
}
/*******************************************************************************
* Function Name  : ConnectServer
* Description    : 链接IP
* Input          :
* Return         :
*******************************************************************************/
void ConnectServer(void)
{
	u8 j;

	if(ConnectIndex == 0)
	{
		EEPROM_OP((u8*)UDP_SERVER,UDP_ADDR1,TCPCHARLEN,MODE_R);
		EEPROM_OP((u8*)TCP_SERVER,TCP_ADDR1,TCPCHARLEN,MODE_R);
		ATCmdFIFO((u8 *)UDP_SERVER,TCPIP_OPEN);
		UARTWrite((u8 *)"ip1\r\n",5,0);

	}
	else if(ConnectIndex == 1)
	{
		//memcopy((u8*)UDP_SERVER,(u8*)UDP_SERVER2,43);
		//memcopy((u8*)TCP_SERVER,(u8*)TCP_SERVER2,43);
		EEPROM_OP((u8*)UDP_SERVER,UDP_ADDR2,TCPCHARLEN,MODE_R);
		EEPROM_OP((u8*)TCP_SERVER,TCP_ADDR2,TCPCHARLEN,MODE_R);
		ATCmdFIFO((u8 *)UDP_SERVER,TCPIP_OPEN);
		UARTWrite((u8 *)"ip2\r\n",5,0);
	}
	else if(ConnectIndex == 4)
	{
		if(ipaddrlen)
		{

			EEPROM_OP((u8*)TCP_SERVER,TCP_ADDR1,TCPCHARLEN,MODE_R);
			for(j = 0;j<15;j++)
				TCP_SERVER[j + 19] = ipaddr[j];
			for(j = 0; j <4;j++)
				TCP_SERVER[j + 36] =  TCP_PORT[j];
			EEPROM_OP((u8*)UDP_SERVER,UDP_ADDR1,TCPCHARLEN,MODE_R);
			for(j = 0;j<15;j++)
				UDP_SERVER[j + 19] = ipaddr[j];
			for(j = 0; j <4;j++)
				UDP_SERVER[j + 36] =  UDP_PORT[j];
			ATCmdFIFO((u8 *)UDP_SERVER,TCPIP_OPEN);
			UARTWrite((u8 *)"DNS\r\n",5,0);
		}
		else
			ConnectIndex = 3;
	}

	ReconnectTimes++;

	if(ReconnectTimes < 3)
	{
		ConnectIndex = 0;
	}
	else if(ReconnectTimes < 6)
	{
		ConnectIndex = 1;
	}
	else if(ReconnectTimes == 6)
	{
		ConnectIndex = 2;
	}

	TCP_ErrorCnt=1;

}
/*******************************************************************************
* Function Name  : DisConnectServer
* Description    : DisConnectServer
* Input          :
* Return         :
*******************************************************************************/
void DisConnectServer(void)
{
	ATCmdFIFO((u8 *)"AT+QICLOSE=0\r\0",TCP_CLOSE);
	ATCmdFIFO((u8 *)"AT+QICLOSE=1\r\0",UDP_CLOSE);
	ATCmdFIFO((u8 *)"AT+QIDEACT\r\0",GPRS_DEACT2);
}

void GPRS_POWER_ON(void)
{
	GPIO_SetBits(GPRS_ON_PORT,GPRS_ON_PIN);
	FEED_WTDG;
	SysTick_Delay_Ms(1000);
	FEED_WTDG;
	SysTick_Delay_Ms(1000);
	GPIO_ResetBits(GPRS_ON_PORT,GPRS_ON_PIN);
	FEED_WTDG;
}
/*******************************************************************************
* Function Name  : NetModulePowerOP
* Description    : 开关机
* Input          :
* Return         :
*******************************************************************************/
void NetModulePowerOP(u8 mode)
{
	switch(mode)
	{
		case SOFT_RESET:
			UARTWrite((u8*)"AT+QPOWD=2\r",11,GPRS_COM); //软重启
			SysTick_Delay_Ms(10);
			MODULE_RESETed=1;
			MODULE_RESETed_Cnt=0;
			break;
		case HD_PWR_ON:
			USART1_Config(57600);
			printf("MON1\r\n");
			ModuleFlagsIni();
			NETFlagsIni();
			MODULE_RESETed=1;
			MODULE_RESETed_Cnt=0;
			GPRS_POWER_ON();
			break;
		case SOFT_PWR_OFF://软件正常关机
			ATCmdFIFO((u8 *)"AT+QPOWD=1\r\0",SOFT_PWR_OFF);
			TCPIP_been_OK=0;
			break;
		case HD_PWR_OFF://硬件正常关机
			printf("GPRS nor off\r\n");
			GPRS_POWER_OFF;
			SysTick_Delay_Ms(5000);//等待VDD输出拉低
			ModuleTempClosedCnt=1;
			TCPIP_been_OK=0;
			MODULE_RESETed=1;
			MODULE_RESETed_Cnt=0;
			break;
		case EM_PWR_OFF://硬件紧急关机
			printf("GPRS eno off\r\n");
			GPRS_POWER_OFF;
			SysTick_Delay_Ms(5000);//等待VDD输出拉低
			ModuleTempClosedCnt=1;
			NetModuleSTATUS=EM_PWR_OFF;
			TCPIP_been_OK=0;
			MODULE_RESETed=1;
		  MODULE_RESETed_Cnt=0;
			break;
		case HD_PWR_OFF2://硬件正常关机2
			GPRS_POWER_OFF;
		  NetModuleSTATUS=M35_KEEPOFF;
			TCPIP_been_OK=0;
			break;
		default:
			break;
	}
}
/*******************************************************************************
* Function Name  : SearchIPchar
* Description    : IP格式对齐000.000.000.000
* Input          :
* Return         :
*******************************************************************************/
u8 SearchIPchar(u8 *paras,u8 *IPaddress,u8 len)
{
	u8 pointsnember=0;
	u8 *p;
	u8	num,j=0,k=0;

	p = paras;
	for(num = 0;num < len; num++)
	{
		if((*paras == 0x0d)&&(*(paras + 1) == 0x0A))
			break;
		if(*(paras + num) == '.')
			pointsnember++;
	}
	if(pointsnember == 3)
	{
		for(num = 0;num < len;num++)
		{
			//*(IPaddress + num) = *(p+num);
			switch(j)
			{
				case 0:
					if(*(paras + num)=='.')
					{
						k = num;
						if(k==3)
						{
							*(IPaddress + 0) = *(p + num -3);
							*(IPaddress + 1) = *(p + num -2);
							*(IPaddress + 2) = *(p + num -1);
						}
						else if(k == 2)
						{
							*(IPaddress + 1) = *(p + num -2);
							*(IPaddress + 2) = *(p + num -1);
						}
						else if(k == 1)
						{
							*(IPaddress + 2) = *(p + num -1);
						}
						j = 1;
					}
					break;
				case 1:
					if(*(paras+num)=='.')
					{
						k=num-k-1;
						if(k==3)
						{
							*(IPaddress + 4) = *(p + num -3);
							*(IPaddress + 5) = *(p + num -2);
							*(IPaddress + 6) = *(p + num -1);
						}
						else if(k==2)
						{
							*(IPaddress + 5) = *(p + num -2);
							*(IPaddress + 6) = *(p + num -1);
						}
						else if(k==1)
						{
							*(IPaddress + 6) = *(p + num -1);
						}
						j=2;
						k=num;
					}
					break;
				case 2:
					if(*(paras+num)=='.')
					{
						k=num-k-1;
						if(k==3)
						{
							*(IPaddress + 8) = *(p + num -3);
							*(IPaddress + 9) = *(p + num -2);
							*(IPaddress + 10) = *(p + num -1);
						}
						else if(k==2)
						{
							*(IPaddress + 9) = *(p + num -2);
							*(IPaddress + 10) = *(p + num -1);
						}
						else if(k==1)
						{
							*(IPaddress + 10) = *(p + num -1);
						}
						j=3;
						k=num;
					}
					break;
				case 3:
					if(*(paras+num)== 0x0d)
					{
						k=num-k-1;
						if(k==3)
						{
							*(IPaddress + 12) = *(p + num -3);
							*(IPaddress + 13) = *(p + num -2);
							*(IPaddress + 14) = *(p + num -1);
						}
						else if(k==2)
						{
							*(IPaddress + 13) = *(p + num -2);
							*(IPaddress + 14) = *(p + num -1);
						}
						else if(k==1)
						{
							*(IPaddress + 14) = *(p + num -1);
						}
						j=0;
						k=0;
					}
					break;
			default:
				break;
			}

		}
		return num;
	}
	else
		return 0;

}
/*******************************************************************************
* Function Name  : AT_Quota_Pos
* Description    : AT_Quota_Pos
* Input          :
* Return         :
*******************************************************************************/
u8 AT_Quota_Pos(u8 *buf,u8 cx)
{
	u8 *p=buf;
	while(cx)
	{
		if(*buf<' '||*buf>'z')return 0XFF;//遇到非法字符,则不存在第cx个逗号
		if(*buf=='\"')cx--;
		buf++;
	}
	return buf-p;
}
/*******************************************************************************
* Function Name  : AT_Comma_Pos
* Description    : AT_Comma_Pos
* Input          :
* Return         :
*******************************************************************************/
u8 AT_Comma_Pos(u8 *buf,u8 cx)
{
	u8 *p=buf;
	while(cx)
	{
		if(*buf<' '||*buf>'z')return 0XFF;//遇到非法字符,则不存在第cx个逗号
		if(*buf==',')cx--;
		buf++;
	}
	return buf-p;
}
/*******************************************************************************
* Function Name  : AT_Asterisk_Pos
* Description    : AT_Asterisk_Pos
* Input          :
* Return         :
*******************************************************************************/
u8 AT_Asterisk_Pos(u8 *buf,u8 cx)
{
	u8 *p=buf;
	while(cx)
	{
		if(*buf<' '||*buf>'z')return 0XFF;//遇到非法字符,则不存在第cx个星号
		if(*buf=='*')cx--;
		buf++;
	}
	return buf-p;
}

/*******************************************************************************
* Function Name  : AT_Colon_Pos
* Description    : AT_Colon_Pos
* Input          :
* Return         :
*******************************************************************************/
u8 AT_Colon_Pos(u8 *buf,u8 cx)
{
	u8 *p=buf;

	while(cx)
	{
		if(*buf<' '||*buf>'z')return 0XFF;//遇到非法字符,则不存在第cx个冒号
		if(*buf==':')cx--;
		buf++;
	}
	return buf-p;
}
/*******************************************************************************
* Function Name  : ATResponseOP
* Description    : 回复AT指令
* Input          :
* Return         :
*******************************************************************************/
void ATResponseOP(u8 *paras)
{
	u16 Dj = 0;
	u8 * p1;
	u8 index = 0;
	u8 showcsq[20];
	u8 colon_index = 0;
	u8 comma_index = 0;
	u8 gprs_time[6];
	
	if((ConnectIndex == 4)&&(AT_Status == DNS_IP))//通过域名解析IP
	{
		if( str_cmp (paras + 2, "OK\0"))
			return;
		ipaddrlen = SearchIPchar(paras + 2,ipaddr,(15+2));
		if(ipaddrlen ==0)
		{
			ConnectIndex = 3;
			UARTWrite((u8 *)"zzzz\r\n",6,0);
		}
		else
		gprsindex = 10;
		ATResponse = AT_RE_OK;
	}
	//UARTWrite((u8*)paras,19,DEBUG_COM);//debuggggggggg
	//-------------取信号强度-------------------//
	if(str_cmp (paras + 2, "+CSQ\0"))//查询模块当前信号值
	{	//+CSQ: 17,99
		//+CSQ: 8,99
		//UARTWrite((u8*)"AT_RCSQ\r\n",9,DEBUG_COM);//debuggggggggg
		//得到CSQ当前数值
		if(*(paras+9)==',')
		{
			Dj=(*(paras+8) & 0x0f);
		}
		else
		{
			Dj = 10*(*(paras+8)&0x0f) + (*(paras+9) & 0x0f);
		}

		CSQValue = Dj;

		if(AT_Status == ASK_CSQ)
		{
			ATResponse = AT_RE_OK;
		}
		memcopy(showcsq,(u8*)"@gprs@CSQ_",10);
		showcsq[10] = CSQValue/10 +0x30;
		showcsq[11] = CSQValue%10 +0x30;
		memcopy((u8*)&showcsq[12],(u8*)"@end@",5);
		UARTWrite(showcsq,17,0);
		SysTick_Delay_Ms(1);
		UARTWrite(showcsq,17,0);
		return;
	}
	//-----------------GSM------------//
	if ( str_cmp (paras + 2, "+CREG: \0"))//联网
	{
		//20150320
		if(Needresetflag[2] != 0x55)
		{
		Needresetflag[2] =0x55;
		EEPROM_OP((u8*)&Needresetflag[2],((u32)(Needresetflag_ADDR)+2),1,MODE_W);//标志位
		}

		if(GpsrCregFlag == 0)
		{	//+CREG: 2,1,"2842","0426" //ASK_CREG
			if((*(paras+11) == '1')||(*(paras+11) == '5'))
			{
				GSMRegisted = 1;
				GSMNORegistCnt = 0;
			}
			else if((*(paras+11) == '0')||(*(paras+11) == '2')||(*(paras+11) == '3'))
			{
				GSMRegisted = 0;
				//20150317
				if(TCPIP_been_OK)
				{
					TCPIP_been_OK = 0;
				}
			}
			ATResponse = AT_RE_OK;
		}
		else if(GpsrCregFlag)
		{	//+CREG: 1,"2842","0461"
			GpsrCregFlag = 0;
			if((*(paras+9) == '1')||(*(paras+9) == '5'))
			{
				GSMRegisted = 1;
				GSMNORegistCnt = 0;
			}
			else if((*(paras+9) == '0')||(*(paras+9) == '2')||(*(paras+9) == '3'))
			{
				GSMRegisted = 0;
				//20150317
				if(TCPIP_been_OK)
				{
					TCPIP_been_OK = 0;
				}
			}
		}
		return;
	}
	//----------------GPRS--------------//
	if ( str_cmp (paras + 2, "+CGREG: \0"))
	{
		//QQ+CGREG: 0,1
		//UARTWrite((u8*)"GPRS REGI",9,DEBUG_COM);//debugg
		if(AT_Status == ASK_CGREG)
		{
			if((*(paras+12) == '1')||(*(paras+12) == '5'))
			{
				//UARTWrite((u8*)"=YES\r\n",6,DEBUG_COM);
				GPRSRegisted = 1;
				GPRSNORegistCnt=0;
			}
			else if((*(paras+12) == '0'))
			{
				//20150320
				//if(GPRSNORegistCnt<600){GPRSNORegistCnt=900;}//1min后重启
				if(GPRSNORegistCnt<120)
				{
					GPRSNORegistCnt=240;	//1min后重启
				}
				GPRSRegisted = 0;
				//20150317
				if(TCPIP_been_OK)
				{
					TCPIP_been_OK = 0;
				}
			}
			else if((*(paras+12) == '2')||(*(paras+12) == '3'))
			{
				GPRSRegisted = 0;
				//20150317
				if(TCPIP_been_OK)
				{
					TCPIP_been_OK = 0;
				}
			}
			ATResponse = AT_RE_OK;
		}
		return;
	}
	if(str_cmp(paras + 2, "+QISTAT: 0\0"))
	{
		if((str_cmp(paras + 11, "0, \"\", \"\"\0"))&&(TCPIP_been_OK&0x01))
		{
			GPRSReConnectFlag = 1;
			UARTWrite((u8 *)"tcpcut\r\n",8,0);
		}
			UARTWrite((u8 *)"tcpg22\r\n",8,0);
		if(AT_Status == ASK_TCPIP)
		{
			ATResponse = AT_RE_OK;
		}
		return;
	}
	if(str_cmp(paras + 4, "+QISTAT: 0\0"))
	{
		if((str_cmp(paras + 13, "0, \"\", \"\"\0"))&&(TCPIP_been_OK&0x01))
		{
			GPRSReConnectFlag = 1;
			UARTWrite((u8 *)"tcpcut\r\n",8,0);
		}
UARTWrite((u8 *)"tcpg44\r\n",8,0);

		if(AT_Status == ASK_TCPIP)
		{
			ATResponse = AT_RE_OK;
		}
		return;
	}
	if(str_cmp(paras + 2, "+QISTAT: 1\0"))
	{
		if((str_cmp(paras + 11, "1, \"\", \"\"\0"))&&(TCPIP_been_OK&0x02))
		{
			GPRSReConnectFlag = 1;
			UARTWrite((u8 *)"udpcut\r\n",8,0);
		}
		if(AT_Status == ASK_TCPIP)
		{
			ATResponse = AT_RE_OK;
		}
		return;
	}
	//+QISACK: 48, 48, 0	12 8+2
	//+QISACK: 0, 0, 0		11	8+2
	if(str_cmp(paras, "+QISACK: \0"))
	{
		p1 = paras;

		index = AT_Comma_Pos(p1,2);   // 16
		colon_index = AT_Colon_Pos(p1,1);	//	8
		comma_index = AT_Comma_Pos(p1,1);	 //   12
		qisacktick = 0;
		if(Tcpsendflag)
		{
			if(comma_index > (colon_index + 2))
			{
				if(*(p1 + colon_index +1) == '0')
					GPRSReConnectFlag = 1;
			}
			if(index > (comma_index + 2))
			{
				if(*(p1 + comma_index +1) == '0')
					Readyrstcnt++;
			}
		}
		if(index != 0xFF)
		{
			if(*(p1 +index +1 ) !='0')
			{
				Readyrstcnt++;
				UARTWrite((u8*)"sss0\r\n",6,DEBUG_COM);
			}
			else
			{
				Readyrstcnt = 0;
				UARTWrite((u8*)"vvv0\r\n",6,DEBUG_COM);

			}
		}
			return;
	}
	if(str_cmp(paras+2, "+QISACK: \0"))
	{
		p1 = paras+2;

		index = AT_Comma_Pos(p1,2);
		colon_index = AT_Colon_Pos(p1,1);
		comma_index = AT_Comma_Pos(p1,1);
		qisacktick = 0;
		if(Tcpsendflag)
		{
			if(comma_index > (colon_index + 2))
			{
				if(*(p1 + colon_index +1) == '0')
					GPRSReConnectFlag = 1;
			}
			if(index > (comma_index + 2))
			{
				if(*(p1 + comma_index +1) == '0')
					Readyrstcnt++;
			}
		}
		if(index != 0xFF)
		{
			if(*(p1 +index +1 ) !='0')
			{
				Readyrstcnt++;
				UARTWrite((u8*)"sss2\r\n",6,DEBUG_COM);
			}
			else
			{
				Readyrstcnt = 0;
				UARTWrite((u8*)"vvv2\r\n",6,DEBUG_COM);

			}
		}
		return;
	}
	if(str_cmp(paras+4, "+QISACK: \0"))
	{
		p1 = paras+4;

		index = AT_Comma_Pos(p1,2);
		colon_index = AT_Colon_Pos(p1,1);
		comma_index = AT_Comma_Pos(p1,1);
		qisacktick = 0;
		if(Tcpsendflag)
		{
			if(comma_index > (colon_index + 2))
			{
				if(*(p1 + colon_index +1) == '0')
					GPRSReConnectFlag = 1;
			}
			if(index > (comma_index + 2))
			{
				if(*(p1 + comma_index +1) == '0')
					Readyrstcnt++;
			}
		}
		if(index != 0xFF)
		{
			if(*(p1 +index +1 ) !='0')
			{
				Readyrstcnt++;
				UARTWrite((u8*)"sss4\r\n",6,DEBUG_COM);
			}
			else
			{
				Readyrstcnt = 0;
				UARTWrite((u8*)"vvv4\r\n",6,DEBUG_COM);
			}
		}
		return;
	}

	//-----SIM检测----------//
	if ( str_cmp (paras + 2, "+CPIN: READY\0"))
	{
		if(Needresetflag[2] != 0x55)
		{
			Needresetflag[2] = 0x55;
			EEPROM_OP((u8*)&Needresetflag[2],((u32)(Needresetflag_ADDR)+2),1,MODE_W);
		}
		UARTWrite((u8*)"@gprs@SIM_OK@end@",17,0);
		SIM_Error_Bit=0;
		ModuleTempClosedCnt=0;
		
		if(cclk_flag==0)//20180305
		{
			ATCmdFIFO((u8 *)"AT+CCLK?\r\0",GPRS_SET);
			cclk_flag=1;
		}
		if(AT_Ready<=1)
		{
			NetModuleBaseIni();AT_Ready=2;
		}
		SIMGeted=1;
		if(AT_Status == ASK_PIN)
		{
			ATResponse = AT_RE_OK;
		}
		ATRE_Null_Cnt=0;
	
		return;
	}

	//if()
	if ( str_cmp (paras + 2, "RDY\0"))
	{
		AT_Ready=1;
		ModuleTempClosedCnt=0;
		if(Needresetflag[2] != 0x55)
		{
			Needresetflag[2] =0x55;
			EEPROM_OP((u8*)&Needresetflag[2],((u32)(Needresetflag_ADDR)+2),1,MODE_W);
		}
		
		return;
	}
	
	if ( str_cmp (paras + 2, "+CCLK: \0"))//20180305
	{
		if(  	(*(paras+10)>=0x30)&&(*(paras+10)<=0x39)&&
					(*(paras+13)>=0x30)&&(*(paras+13)<=0x39)&&
					(*(paras+16)>=0x30)&&(*(paras+16)<=0x39)&&
					(*(paras+19)>=0x30)&&(*(paras+19)<=0x39)&&
					(*(paras+22)>=0x30)&&(*(paras+22)<=0x39)&&
					(*(paras+25)>=0x30)&&(*(paras+25)<=0x39)  )
		{
			gprs_time[0]=(*(paras+10)-0x30)*16+(*(paras+11)-0x30);
			gprs_time[1]=(*(paras+13)-0x30)*16+(*(paras+14)-0x30);
			gprs_time[2]=(*(paras+16)-0x30)*16+(*(paras+17)-0x30);
			gprs_time[3]=(*(paras+19)-0x30)*16+(*(paras+20)-0x30);
			gprs_time[4]=(*(paras+22)-0x30)*16+(*(paras+23)-0x30);
			gprs_time[5]=(*(paras+25)-0x30)*16+(*(paras+26)-0x30);
			RTC_OP(gprs_time,RTC_SETTIME_BCD);
		}
		else if(rtc_updata_flag==0)
			ATCmdFIFO((u8 *)"AT+CCLK?\r\0",GPRS_SET);
		
		return;
	}
	if ( str_cmp (paras + 2, "+CPIN: NOT READY\0"))
	{
		ModuleNeedCutRST= 1; //2015.10.13
		if(Needresetflag[2] != 0x55)
		{
			Needresetflag[2] =0x55;
			EEPROM_OP((u8*)&Needresetflag[2],((u32)(Needresetflag_ADDR)+2),1,MODE_W);
		}

		UARTWrite((u8*)"@gprs@NOT_SIM@end@",18,0);

		return;
	}
	if ( str_cmp (paras + 2, "Call Ready\0"))
	{
		if(Needresetflag[2] != 0x55)
		{
			Needresetflag[2] =0x55;
			EEPROM_OP((u8*)&Needresetflag[2],((u32)(Needresetflag_ADDR)+2),1,MODE_W);
		}
		
		if(AT_Ready<2)
		{
			NetModuleBaseIni();SIMGeted=1;
		}
		AT_Ready=5;
		UARTWrite((u8*)"ATE0\r\0",5,GPRS_COM);
		SIM_Error_Bit=0;
		ModuleTempClosedCnt=0;
		GPRSIni();

		return;
	}
	if( str_cmp (paras + 2, "+QIMUX: \0"))
	{
		if( *(paras+10) == '1')
		{
			gpsriniflag = 1;
			qimuxflag = 1;
		}
		else
		{
			ATCmdFIFO((u8 *)"AT+QIMUX=1\r\0",GPRS_SET);
			SysTick_Delay_Ms(10);
			ATCmdFIFO((u8 *)"AT+QIMUX?\r\0", GPRS_SET);
		}
		return;
	}
	if( str_cmp (paras + 2, "+QNITZ: \0"))
	{
		if(( *(paras+10) == '1')||( *(paras+10) == '\"'))
		{
			ATCmdFIFO((u8 *)"AT+CCLK?\r\0",GPRS_SET);//gpsriniflag = 1;更新网络时间后，再次获取最新时间，防止时间误差较大
		}
		else
		{
			ATCmdFIFO((u8 *)"AT+QNITZ=1\r\0",GPRS_SET);
			SysTick_Delay_Ms(10);
			ATCmdFIFO((u8 *)"AT+QNITZ?\r\0", GPRS_SET);
		}
		return;
	}
	if( str_cmp (paras + 2, "+CTZU: \0"))
	{
		if( *(paras+9) == '3')
		{
			;//gpsriniflag = 1;
		}
		else
		{
			ATCmdFIFO((u8 *)"AT+CTZU=3\r\0",GPRS_SET);
			SysTick_Delay_Ms(10);
			ATCmdFIFO((u8 *)"AT+CTZU?\r\0", GPRS_SET);
		}
		return;
	}
	//-------TCP,UDP连接建立成功-------------//
	if ( str_cmp (paras + 2, "0, CONNECT OK\0"))
	{
		NetModuleSTATUS=TCPIP_ONLINE;
		TCPIP_been_OK&=~0X01;
		TCPIP_been_OK|=0X01;
		if(AT_Status == TCPIP_OPEN)
		{
			ATResponse = AT_RE_OK;
		}
		HandTick= (DevParameters.HandTick*4)-40;//10S后发送握手信号//
		GSMCHECKcnt=GSM_CHECK_PERIOD-20;//5S
		TCP_ErrorCnt=0;
		TCP_ErrorCnt1=0;
		ReconnectTimes = 0;
		ModuleTempClosedCount = 0;
		GPRSReConnectCnt=0;

		UARTWrite((u8*)"@gprs@TCP_OK@end@",17,0);
		NewPacket_KY((u8*)&(DevParameters.lince[1]),DevParameters.lince[0],DEVICE_AUTHEN,TCPLINK);
		return;
	}
	if ( str_cmp (paras + 2, "1, CONNECT OK\0"))
	{
		NetModuleSTATUS=TCPIP_ONLINE;
		TCPIP_been_OK&=~0X02;
		TCPIP_been_OK|=0X02;
		if(AT_Status == TCPIP_OPEN)
		{
			ATResponse = AT_RE_OK;
		}
		//ATCmdFIFO((u8 *)"AT+QIOPEN=0,\"TCP\",\"59.42.254.248\",2008\r\0",TCPIP_OPEN);
		UARTWrite((u8*)"@gprs@UDP_OK@end@",17,0);
		ATCmdFIFO((u8 *)TCP_SERVER,TCPIP_OPEN);
		return;
	}
	if ( str_cmp (paras + 2, "0, CLOSE OK\0"))
	{
		NetModuleSTATUS=IPCLOSE;
		TCPIP_been_OK&=~0X01;
		ATResponse = AT_RE_OK;
		return;
	}
	if ( str_cmp (paras + 2, "1, CLOSE OK\0"))
	{
		NetModuleSTATUS=IPCLOSE;
		TCPIP_been_OK&=~0X02;
		ATResponse = AT_RE_OK;
		return;
	}
	if ( str_cmp (paras + 2, "0, CONNECT FAIL\0"))
	{
		//if((TCPIP_been_OK&0X01)!=0x01)
		{
			NetModuleSTATUS=IPCLOSE;
			//	ATCmdFIFO((u8 *)TCP_SERVER,TCPIP_OPEN);
			GPRSReConnectCnt++;
			//ReconnectTimes++;
		}
		if(AT_Status == TCPIP_OPEN)
		{
			ATResponse = AT_RE_OK;
		}
		return;
	}
	if ( str_cmp (paras + 2, "1, CONNECT FAIL\0"))
	{
		//if((TCPIP_been_OK&0X02)!=0x02)
		{
			NetModuleSTATUS=IPCLOSE;
			//ATCmdFIFO((u8 *)UDP_SERVER,TCPIP_OPEN);
			GPRSReConnectCnt++;
			//ReconnectTimes++;
		}
		if(AT_Status == TCPIP_OPEN)
		{
			ATResponse = AT_RE_OK;
		}
		return;
	}
	if ( str_cmp (paras + 2, "DEACT OK\0"))
	{
		NetModuleSTATUS=IPINI;
		TCPIP_been_OK=0;
		if((AT_Status == GPRS_DEACT)||(AT_Status == GPRS_DEACT2))
		{
			ATResponse = AT_RE_OK;
		}
		return;
	}
	if ( str_cmp (paras + 2, "STATE:\0"))
	{
		if((gprsindex == 2)&&(str_cmp (paras + 9, "IP INITIAL\0")))  //STATE: IP INITIAL
		{
			gprsindex = 3;
		}
		if((gprsindex == 5)&&(str_cmp (paras + 9, "IP START\0")))  //STATE: IP START
		{
			gprsindex = 6;
		}
		if((gprsindex == 8)&&(str_cmp (paras + 9, "IP GPRSACT\0")))  //STATE: IP START
		{
			gprsindex = 9;
		}
		if((gprsindex == 11)&&(str_cmp (paras + 9, "IP STATUS\0")))  //STATE: IP START
		{
			gprsindex = 12;
		}
		//if(AT_Status == GPRS_QISTATE)
		ATResponse = AT_RE_OK;
		return;
	}
	if(((*(paras+2) == '0')||(*(paras+2) == '1')||(*(paras+2) == '2')
	||(*(paras+2) == '3')||(*(paras+2) == '4')||(*(paras+2) == '5')
	||(*(paras+2) == '6')||(*(paras+2) == '7')||(*(paras+2) == '8')
	||(*(paras+2) == '9'))
	&&((*(paras+3) == '0')||(*(paras+3) == '1')||(*(paras+3) == '2')
	||(*(paras+3) == '3')||(*(paras+3) == '4')||(*(paras+3) == '5')
	||(*(paras+3) == '6')||(*(paras+3) == '7')||(*(paras+3) == '8')
	||(*(paras+3) == '9')||(*(paras+3) == '.')))
	{
		//UARTWrite((u8*)"111111\r\n",8,0);
		ATResponse = AT_RE_OK;
		return;
	}
	if ( str_cmp (paras + 2, "0, CLOSED\0"))
	{
		NetModuleSTATUS=IPCLOSE;
		if(TCPIP_been_OK)
		{
			TCPIP_been_OK&=~0X01;
			GPRSReConnectFlag=1;
		}
		return;
	}
	if ( str_cmp (paras + 2, "1, CLOSED\0"))
	{
		NetModuleSTATUS=IPCLOSE;
		if(TCPIP_been_OK)
		{
			TCPIP_been_OK&=~0X02;
			GPRSReConnectFlag=1;
		}
		return;
	}
	if ( str_cmp (paras + 2, "+PDP DEACT\0"))
	{
		NetModuleSTATUS=GPRS_DEACT;
		if(TCPIP_been_OK)
		{
			TCPIP_been_OK=0;GPRSReConnectFlag=1;
		}
		return;
	}
	if ( str_cmp (paras + 2, "SEND FAIL\0"))
	{
		TCPIP_been_OK=0;
		GPRSReConnectFlag=1;
		return;
	}
	if ( str_cmp (paras + 4, "SEND FAIL\0"))
	{
		TCPIP_been_OK=0;
		GPRSReConnectFlag=1;
		return;
	}
	//----------------GET A CALL--------------//
	//RING	来电振铃////+CLIP: "",128来电显示//
	if ( str_cmp (paras + 2, "RING\0"))
	{
		return;
	}
	if ( str_cmp (paras + 2, "+CLIP:\0"))
	{
		return;
	}
	if ( str_cmp (paras + 2, "MO CONNECTED\0"))
	{
		return;
	}

	if ( str_cmp (paras + 2, "+CCLK:\0"))
	{
		//+CCLK: "YY/MM/DD,hh:mm:ss+zz"
		//+CCLK: "00/01/01,00:02:14+08"
//		for(Di=0;Di<6;Di++)
//		{
//			NowTimeBuf[Di]=(paras[3*Di+10]-0x30)<<4 | (paras[3*Di+11]-0x30);
//		}
//		SendData2TestCOM(NowTimeBuf,6,0);

		ATResponse = AT_RE_OK;
		return;
	}
	//------------------是否收到OK----------------//
	if(str_cmp (paras + 2, "OK\0"))
	{
		ATResponse = AT_RE_OK;

		//LEDshow('G');LEDshow('R');//DEBUGGGG
		//LastATSending = 0;//冗余码//
		//UARTWrite((u8*)"\r\nAT_ROK\r\n",10,DEBUG_COM);//debuggggggggg
		if(AT_Status == SET_FUNC)
		{
			SysTick_Delay_Ms(100);

		}
		else if(AT_Status == SET_PIN1)
		{
			ATCmdFIFO((u8 *)"AT+CPWD=\"SC\",\"1234\",\"0318\"\r\0",SET_PIN2);
		}
		else if(AT_Status == SET_PIN5)
		{
			ATCmdFIFO((u8 *)"AT+CLCK=\"SC\",0,\"1234\"\r\0",SET_PIN6);
		}
		else if(AT_Status == SET_CCLK)
		{
			//shoushiok//
		}
		ATRE_Null_Cnt=0;
		return;
	}
	//---------------是否收到ERROR---------------//
	if ( str_cmp (paras+2, "ERROR\0"))
	{
		ATResponse = AT_RE_ERR;
		//UARTWrite((u8*)"\r\nAT_RERR\r\n",11,DEBUG_COM);//debuggggggggg
		if((AT_Status>=SET_PIN0)&&(AT_Status<=IN_PIN))
		{
			LastATSending = 0;
			ATCmdListPushUp();
		}
		else if((GPRS_DEACT==AT_Status)||(SET_CCLK==AT_Status))
		{
			LastATSending = 0;
			ATCmdListPushUp();
		}
		else if((GPRS_QIREFAPP==AT_Status)||(GPRS_QIACT==AT_Status)||(GPRS_QILOCIP==AT_Status))
		{
			LastATSending = 0;
			ATCmdListPushUp();
			GPRSReConnectFlag = 1;
		}
		else if(TCPIP_OPEN==AT_Status)
		{
			ModuleNeedCutRST = 1;
		}

		/*if( paras[7]==':' )
		{
		if((paras[10]<'0')||(paras[10]>'9')) {Di=paras[9]-0x30;}
		else { Di=(paras[9]-0x30)*10+(paras[10]-0x30);}
		}*/
		ATRE_Err_Cnt++;
		ATRE_Null_Cnt = 0;
		SysTick_Delay_Ms(5);
		return;
	}

	//-------------------------------------------//
	if ( str_cmp (paras+2, "+CME ERROR:\0") )
	{
		ATResponse = AT_RE_ERR;
		ATRE_Err_Cnt++;
		ATRE_Null_Cnt = 0;
		//UARTWrite((u8*)"\r\nAT_RERR\r\n",11,DEBUG_COM);//debuggggggggg
		if((AT_Status>=SET_PIN0)&&(AT_Status<=IN_PIN))
		{
			LastATSending = 0;
			ATCmdListPushUp();
		}
		if ( str_cmp (paras+2, "+CME ERROR: 13\0"))
		{
			LastATSending = 0;
			SIM_Error_Bit = 1;
			ATRE_Err_Cnt+=6;
		}
		else if ( str_cmp (paras+2, "+CME ERROR: 10\0"))
		{
			SIMGeted=0;SIM_Error_Bit=0;
			ATRE_Err_Cnt++;//ATRE_Err_Cnt=22;
			//UARTWrite((u8*)"SIM_NOT ",8,DEBUG_COM);//debuggggggggg
			SysTick_Delay_Ms(5);//UARTWrite((u8*)"AT+CNUM\r\0",8,GPRS_COM);
		}
		else if(str_cmp(paras+2, "+CME ERROR: 11\0"))
		{
			SIMGeted=0;SIM_Error_Bit=0;
			ATRE_Err_Cnt++;//ATRE_Err_Cnt=22;
			UARTWrite((u8*)"AT+CPIN=\"0318\"\r",15,GPRS_COM);
		}
		else if ( str_cmp (paras+2, "+CME ERROR: 22\0"))
		{
			SIMGeted=0;
			SIM_Error_Bit=0;
			ATRE_Err_Cnt++;
			if(ATRE_Err_Cnt==10)
			{
				LastATSending = 0;
				ATCmdListPushUp();
				//ATCmdFIFO((u8 *)"AT+CPBS=\"ON\"\r\0",SET_CPBS);//090301-
				//ATCmdFIFO((u8 *)"AT+CPBW=1,\"13620092009\",129,\"TT\"\r\0",SET_CPBS);//090301-
			}
		}
		SysTick_Delay_Ms(5);
		return;
	}
	//-----------------------------------------//
	if (str_cmp (paras+2, "+CMS ERROR:\0"))
	{
		ATResponse = AT_RE_ERR;
		//UARTWrite((u8*)"\r\nAT_RE_ERR\r\n",13,DEBUG_COM);//debuggggggggg
		ATRE_Err_Cnt+=6;
		ATRE_Null_Cnt = 0;
		return;
	}
	if (str_cmp (paras+2, "UNDER_VOLTAGE POWER DOWN\0"))
	{
		NetModuleSTATUS=LOWVDD_PWR_OFF;
		HadCut_MPW_BOLV=1;
		GPS_SW(0);
		return;
	}

}

/*******************************************************************************
* Function Name  : ATCmdFIFO
* Description    : 将要发送的AT指令存入队列，同时取队列指针，将要发送的AT指令放入正确对应指针//
//存放规则：判断最后一个头上有'&'的指针位，将要增加的放入。//
* Input          :
* Return         : 1OK,0表示存放区域已满，无法执行操作//
*******************************************************************************/
void ATCmdFIFO(u8 * cmdbuf, u8	cmdtype)
{
	u8	i = 0;

	for(i = 0;i < AT_CMD_CNT;i++)
	{
		if((AT_CMD_list[i][0] != GPRS_ID)&&(AT_CMD_list[i][0] != AT_ID))
		{
			break;
		}
	}

	if(i == AT_CMD_CNT)
	{
		UARTWrite((u8*)"AT_CNT_ERR\r\n",12,DEBUG_COM);//debuggggggggg
		return;
	}

	str_cpy (&AT_CMD_list[i][2],cmdbuf);
	AT_CMD_list[i][0] = AT_ID;
	AT_CMD_list[i][1] = cmdtype;

	//UARTWrite((u8*)&AT_CMD_list[i][0],12,DEBUG_COM);//debuggggggggg
	return;
}
//=========================================================================//
//函数名：ATCmdSend(void)//
//用途：将AT指令队列内容送到GPRSCOM//
//判断条件：前次的AT发送是否成功//
//后续工作：在收到相应的AT回应后//
//将下一帧上移，下下帧跟随//
//返回值//
//0表示上次发送的指令未完成，1表示无数据可发，无法执行操作//
//=========================================================================//
u8	ATCmdSend(u8 taskid)
{
	u8  at_cmdsend_strbuff[80];
	u8 i,reid;

	if(ModuleNeedCutRST)
	{
		return 0;
	}

	if(LastATSending != 0)
	{
		return ( 0 );
	}

	if((AT_CMD_list[0][0] != GPRS_ID)&&(AT_CMD_list[0][0] != AT_ID))
	{
		return ( 1 );
	}

	if(taskid==NO_ID)
	{
		reid=AT_CMD_list[0][0];
		i=0;
	}
	else
	{
		for(i = 0;i < AT_CMD_CNT;i++)
		{
			if(AT_CMD_list[i][0]== taskid)
			{
				reid=taskid;
				break;
			}
		}
	}

	if(i>=AT_CMD_CNT)
	{
		return ( 1 );
	}


	str_cpy (at_cmdsend_strbuff,&AT_CMD_list[i][2] );
	str_cat (at_cmdsend_strbuff, "\0" );


	ATcmdEXE(at_cmdsend_strbuff,AT_CMD_list[i][1]);

	//------------------------------//
	at_cmdsend_strbuff[0]=i;
	if(at_cmdsend_strbuff[0]!=0)
	{
		str_cpy (&at_cmdsend_strbuff[1],&AT_CMD_list[i][0] );//back//

		for(i=at_cmdsend_strbuff[0];i>0;i--)
		{
			str_cpy (&AT_CMD_list[i][0],&AT_CMD_list[i-1][0] );
		}

		str_cpy (&AT_CMD_list[0][0],&at_cmdsend_strbuff[1]);
	}
	return (reid);
}
//=========================================================================//
//函数名：ATCmdListPushUp								                //
//用途：将已经发送成功的指令清空，将未发送的所有指令依次上移	//
//使用时放在接收回码程序的最后										            //
//=========================================================================//
u8	ATCmdListPushUp(void)
{
	u8	 i = 0,j = 0;

	if(LastATSending)
		return ( 0 );		//发送还没完成//
	for(i = 0;i < AT_CMD_CNT;i++)
	{
		if((AT_CMD_list[i][0] != AT_ID)&&(AT_CMD_list[i][0] != GPRS_ID))
		{
			break;
		}
	}

	if(i == 0)
		return ( 1 );		//没有要推进的AT指令//

	if(i == 1)
	{
		AT_CMD_list[0][0] = 0;
		return ( 2 );	//已经将当前发送的AT指令清空//
	}

	for(j = 0;j<(i-1);j++)
	{
		str_cpy (&AT_CMD_list[j][0],&AT_CMD_list[j+1][0] );
		AT_CMD_list[j+1][0] = 0;
	}
	return ( 3 );			//有多条消息，将上条消息推入//
}
/*******************************************************************************
* Function Name  : ATcmdEXE
* Description    : 往后台发AT指令
* Input          :
* Return         :
*******************************************************************************/
u8 ATcmdEXE(u8 *atstr,u8 type)
{
	//将命令发到串口//
	u8 sendlenth;

	sendlenth=str_len(atstr);

	UARTWrite(atstr,sendlenth,GPRS_COM);
	SysTick_Delay_Ms(5);
	UARTWrite(atstr,sendlenth,DEBUG_COM);//

	ATResponse  = 0;
	LastATSending = 1;
	AT_Status = type;//AT_CMD_list[0][1];

	//设置回码检测定时器，一般5S//
	//SMS发送最长等待时间为60秒//
	ATRe_WaitTick = 20;
	switch(type)
	{
		case SET_FUNC:
		case SET_ATE:
		case ASK_PIN:
			ATRe_WaitTick = 60;
			break;

		case TCPIP_OPEN:
		//case UDP_OPEN:
		//case TCP_CLOSE:
		//case UDP_CLOSE:
		case EXIT_TCPIP:
		case DNS_IP:
		case GPRS_QIACT:
			ATRe_WaitTick = 152;
			break;

		case TCP_SEND:
		case UDP_SEND:
			ATRe_WaitTick = 40;
			break;

		case SET_STORE:
			ATRe_WaitTick = 20;
			break;

		case GPRS_DEACT:
		case GPRS_DEACT2:
			ATRe_WaitTick = 240;
			break;

		default:
			break;
	}
	return(0xff);
}
/*******************************************************************************
* Function Name  : ModuleFlagsIni
* Description    : ModuleFlagsIni
* Input          :
* Return         :
*******************************************************************************/
void ModuleFlagsIni(void)
{
	u8 i;

	LastATSending = 0;
	AT_Status = 0;
	ATResponse = 0;
	ATRE_Err_Cnt = 0;
	ATSend_Err_Cnt = 0;
	ATSend_Err_Cnt11 = 0;
	ATRE_Null_Cnt = 0;
	UART1_TASK_STATUS = IDLE_STATUS;
	NetModuleSTATUS = 0;
	UART1_TASK_ID =	NO_ID;

	AT_Ready=0;
	Tcpsendflag = 0;
	tcpsendtick = 0;
	qisacktick = 0;
	SIM_Error_Bit = 0;
	SIM_Error_Cnt = 0;
	SIMGeted=0;

	GSMRegisted = 0;
	GSMNORegistCnt = 0;
	GPRSRegisted=0;
	GPRSNORegistCnt =0;
	TCP_ErrorCnt=0;
	TCP_ErrorCnt1= 0;
	GPRSReConnectFlag=3;
	GPRSReConnectCnt=0;
	ReconnectTimes = 0;
	ConnectIndex = 0;
	GpsrCregFlag =0;
	gprsindex = 0;
	gpsriniflag = 0;
	qimuxflag = 0;
	Readyrstcnt = 0;
	Lowpowertick = 0;
	ModuleNeedCutRST=0;
	MODULE_RESETed=0;
	MODULE_RESETed_Cnt=0;
	ModuleTempClosedCnt=0;
	GSMCHECKcnt=0;
	TCPCHECKcnt =0;
	GPRS_NET_TIPtick=0;


  for(i = 0;i <SMS_RECBUF_CNT;i++)
  	{
		SMS_RCV_Buff[i][0] = 0;
	}

}
/*******************************************************************************
* Function Name  : NETFlagsIni
* Description    : 清除AT临时BUF内容
* Input          :
* Return         :
*******************************************************************************/
void NETFlagsIni(void)
{
	u8 i;
  	//AT发送列表//
	for(i = 0;i<AT_CMD_CNT;i++)
	{ AT_CMD_list[i][0] = 0; }
	//AT接收列表//
	for(i = 0;i<AT_INBUF_CNT;i++)
 // {	*(AT_RCV_Buff+i*AT_INBUF_SIZE) = 0; }
   {	AT_RCV_Buff[i][0] = 0;}
	//IP接收列表
	for(i = 0;i<IP_RECBUF_CNT;i++)
	//{*(IP_RCV_Buff+i*IPBUF_SIZE) = 0;}
	{IP_RCV_Buff[i][0]=0;}//
	TCPIP_been_OK=0;
}
/*******************************************************************************
* Function Name  : Module_TimerHandler
* Description    : 每250MS调用一次，针对AT和MODULE的控制
* Input          :
* Return         :
*******************************************************************************/
void Module_TimerHandler(void)
{
	//u8 i;

	if(ATRe_WaitTick > 1)
	{
		ATRe_WaitTick--;
	}
	if(ModuleTempClosedCnt)
	{
		//MODULE掉电延时重新上电//
		ModuleTempClosedCnt++;
		if(ModuleTempClosedCnt >=60)   //   15s
		{
			printf("M26 restart\r\n");
			ModuleTempClosedCnt=0;
			NetModulePowerOP(HD_PWR_ON);
		}

		if(TCPIP_been_OK)
		{
			ModuleTempClosedCnt=0;
		}
	}
	if(MODULE_RESETed==1)
	{
		MODULE_RESETed_Cnt++;
		if(MODULE_RESETed_Cnt>=80)     //20s     20150413
		{
			MODULE_RESETed_Cnt=0;
			MODULE_RESETed='Y';
			GPRS_NET_TIPtick=116;
			GSMCHECKcnt=GSM_CHECK_PERIOD-120;//30S
			GSMNORegistCnt=0;
			GPRSNORegistCnt=0;
			if(AT_Ready == 0)
			{
				UARTWrite((u8 *)"RESET\r\n",7,0);
				ModuleNeedCutRST= 1;
			}
		}
	}
	//-------------------------------------//
	if(TCP_ErrorCnt)
	{
		TCP_ErrorCnt++;
		if(TCP_ErrorCnt >= 64)    //连接16s超时
		{
			TCP_ErrorCnt=0;
			GPRSReConnectFlag=1;
		}
	}
	if(ModuleTempClosedCount > 5)
	{
		ModuleTempClosedCount = 0;
		TCPIP_been_OK=0;

		UARTWrite((u8 *)"EM_F\r\n",6,0);//debugggg
		NetModulePowerOP(EM_PWR_OFF);//关机//
	}
	if(ATRE_Null_Cnt>2)
	{
		if(AT_Ready==0)
		{
			TCPIP_been_OK=0;
			NetModulePowerOP(EM_PWR_OFF);//关机//
			UARTWrite((u8 *)"GE1\r\n",5,0);//debugggg
			ATRE_Null_Cnt=0;
			//while(1);
		}
		if(ATRE_Null_Cnt>5)              //无回复 5 次20150317
		{
			UARTWrite((u8 *)"MOFF\r\n",6,0);//debugggg
			TCPIP_been_OK=0;
			NetModulePowerOP(EM_PWR_OFF);//关机//
			UARTWrite((u8 *)"GE2\r\n",5,0);//debugggg
			ATRE_Null_Cnt=0;
			//while(1);
		}
	}

	if(GPRSReConnectCnt>25)
	{
		GPRSReConnectCnt=0;
		ModuleNeedCutRST = 1;
	}

  //AT返回连续错误20次，重启模块//
	if(ATRE_Err_Cnt>=20)
	{
		ATRE_Err_Cnt = 0;
		ModuleNeedCutRST=1;
	}
	//连续发送失败5次，重启模块...
	if(ATSend_Err_Cnt >= 5)
	{
		ATSend_Err_Cnt = 0;
		ModuleNeedCutRST=1;
	}
	if(ATSend_Err_Cnt11 >= 5)
	{
		ATSend_Err_Cnt11 = 0;
		ModuleNeedCutRST=1;
	}
	//连续2分钟GSM没注册，重启模块//
	if(GSMRegisted==0)
	{
		GSMNORegistCnt++;
		if(GSMNORegistCnt>=240)   // //   1min GSM未注册 20150317
		{
			GSMNORegistCnt = 0;
			ModuleNeedCutRST=1;
			printf("GSM未注册！\r\n");
		}
	}

	//连续nsGPRS没注册，重启模块//
	if(GPRSRegisted==0)
	{
		GPRSNORegistCnt++;
		if(GPRSNORegistCnt>= 360) // 1.5min GPRS未注册..
		{
			GPRSNORegistCnt=0;
			ModuleNeedCutRST=1;
			printf("GPRS未注册！\r\n");
		}
	}

	//SIM错误控制//
	if(SIM_Error_Bit == 1)
	{
		if(++SIM_Error_Cnt>=320)
		{
			SIM_Error_Cnt = 0;
			SIM_Error_Bit = 0;
			ModuleNeedCutRST = 1;
		}
	}

	//连续2分钟离线，重启模块//
	if((TCPIP_been_OK&0X03) != 0X03)
	{
		TCP_ErrorCnt1++;
		if(TCP_ErrorCnt1>=480)	//2min
		{
			TCP_ErrorCnt1=0;
			ModuleNeedCutRST= 1;
			UARTWrite((u8 *)"TCPIP00\r\n",9,0);
		}
	}
	
	if(Readyrstcnt >= 3)
	{
		Readyrstcnt = 0;
		GPRSReConnectFlag = 1;
		UARTWrite((u8 *)"TCPIPCUT\r\n",10,0);
	}
	
  //重新启动模块//
	if(ModuleNeedCutRST==1)
	{
		NetModulePowerOP(HD_PWR_OFF);//硬关机//
		ModuleNeedCutRST=0;
		ModuleTempClosedCount++;
	}
	else if(ModuleNeedCutRST==2)
	{
		NetModulePowerOP(EM_PWR_OFF);//紧急关机//
		ModuleNeedCutRST=0;
	}
	
  //-------------------------
	GSMCHECKcnt++;
	if(GSMCHECKcnt>=GSM_CHECK_PERIOD)	//1MIN
	{
		//GSMCHECKcnt = 0;
		GSMCHECKcnt = 120;			 //  30s
		if((MODULE_RESETed=='Y')&&(AT_Ready != 0))
		{
			GSMMCheck();
		}
	}
	
	if((TCPIP_been_OK&0X03)==0X03)
	{
		TCPCHECKcnt++;
		if(TCPCHECKcnt >= 36)
		{
			TCPCHECKcnt = 0;
			ATCmdFIFO((u8 *)"AT+QISACK=0\r\0",ASK_TCPIP);
		}
	}
	Module_ini_forGPRS();
}
/*******************************************************************************
* Function Name  : AT_TASK
* Description    :
//判断是否有AT回码和IP数据及相关处理
//判断是否有AT命令需要发送
//判断AT回码是正确还是错误还是延时态
* Input          :
* Return         :
*******************************************************************************/
void	AT_TASK(void)
{
	u8 i;

	for(i = 0;i<AT_INBUF_CNT;i++)
	{
		if(AT_RCV_Buff[i][0] == 'R')
		{
			ATResponseOP((u8*)&AT_RCV_Buff[i][2]);
			AT_RCV_Buff[i][0]=0;
			break;
		}
	}
  	//判断IP数据
	for(i = 0;i<IP_RECBUF_CNT;i++)
	{
		if(IP_RCV_Buff[i][0] == 'R')
		{
			IPGetOP((u8 *)&IP_RCV_Buff[i][2] , IP_RCV_Buff[i][1] );
			IP_RCV_Buff[i][0]=0;
			break;
		}
	}
	for(i = 0;i<SMS_RECBUF_CNT;i++)
	{
		if(SMS_RCV_Buff[i][0] == 'R')
		{
			SMS_RCV_Buff[i][0]=0;
			if((SMS_RCV_Buff[i][1] > 6)&&(SMS_RCV_Buff[i][1] <150))
				SMSDataOP((u8 *)&SMS_RCV_Buff[i][2],SMS_RCV_Buff[i][1]);
		}
	}
	//===================================
  	//需要对GPRS进行连接操作//
	if((GPRSReConnectFlag == 1)||(GPRSReConnectFlag == 2))
	{
		UART1_TASK_STATUS = BUSY_STATUS;UART1_TASK_ID =	GPRS_ID;
		NETFlagsIni();
		TCPIP_been_OK=0;
		TCP_ErrorCnt1=0;
		TCPCHECKcnt = 0;
		Tcpsendflag = 0;
		GPRS_NET_TIPtick=112;
		if(GPRSReConnectFlag==1) {GPRSReConnectCnt++;}
		GPRSReConnectFlag=0X03;
		gprsindex = 0;
		Module_ini_forGPRS();
	}

	if((UART1_TASK_STATUS == IDLE_STATUS)||(UART1_TASK_ID == AT_ID)||(UART1_TASK_ID ==	GPRS_ID))
	{
		if((UART1_TASK_ID ==	AT_ID)||(UART1_TASK_ID ==	GPRS_ID))
		{
			//判断AT命令回码状态//
			if(ATResponse == AT_RE_OK)
			{
				switch(AT_Status)
				{
					case TCP_CLOSE:
						TCPIP_been_OK&=~0X01;
						break;

					case UDP_CLOSE:
						TCPIP_been_OK&=~0X02;
						break;

					default:
						break;
				}
				AT_Status = AT_INI;
				ATResponse=0;
				LastATSending = 0;
				ATRe_WaitTick = 0;
				ATCmdListPushUp();
				UART1_TASK_STATUS = IDLE_STATUS;
				UART1_TASK_ID =NO_ID;
			}
			else if((ATRe_WaitTick <= 1)||(ATResponse == AT_RE_ERR))
			{
				if(ATRe_WaitTick <= 1)
				{
					ATResponse = AT_RE_NULL;
					ATRE_Null_Cnt++;
					if(ATRE_Null_Cnt==3)//090116+
					{
						UARTWrite((u8*)"ATE0\r\0",5,GPRS_COM);
						if((AT_Status == TCP_SEND)||(AT_Status == UDP_SEND))
						{
							ATCmdListPushUp();
						}
					}
				}
				LastATSending = 0;
				ATRe_WaitTick = 0;
				UART1_TASK_STATUS = IDLE_STATUS;
				UART1_TASK_ID =NO_ID;
				AT_Status = AT_INI;
				ATResponse = 0;
			}
		}

		i=ATCmdSend(UART1_TASK_ID);
		if((i ==	AT_ID)||(i ==	GPRS_ID))
		{
			UART1_TASK_STATUS = BUSY_STATUS;
			UART1_TASK_ID =	i;
		}
	}
}


/*******************************************************************************
* Function Name  : GSMMCheck
* Description    : 查状态  信号值，联网等
* Input          :
* Return         :
*******************************************************************************/
void GSMMCheck(void)
{
	if(qimuxflag == 0)
	{
		ATCmdFIFO((u8 *)"AT+QIMUX?\r\0", GPRS_SET);
	}
	ATCmdFIFO((u8 *)"AT+CSQ\r\0",ASK_CSQ);
	ATCmdFIFO((u8 *)"AT+CREG?\r\0",ASK_CREG); //20150321
	GpsrCregFlag = 0;
	if(GSMRegisted == 1)
	{
		ATCmdFIFO((u8 *)"AT+CGREG?\r\0",ASK_CGREG);
		if(GPRSRegisted==1)
		{
			if(AT_Ready==5)
			{
				SMSIni();
				ATCmdFIFO((u8 *)"AT&W\r\0",SET_STORE);
				//ATCmdFIFO((u8 *)"AT+QSCLK=1\r\0",QSCLK_SET);
				AT_Ready=6;
			}
			if(TCPIP_been_OK)
			{
				ATCmdFIFO((u8 *)"AT+QISTAT\r\0",ASK_TCPIP);
//ATCmdFIFO((u8 *)"AT+QISACK=0\r\0",ASK_TCPIP);
			}
		}
	}
}

/*******************************************************************************
* Function Name  : IPGetOP
* Description    : 获取后台发过来的数据  7e
* Input          :
* Return         :
*******************************************************************************/
void IPGetOP(u8 * ipbuf,u8 iplen)
{
	if(iplen > 2)
	{
		if(ipbuf[0]!=0x7E)
			return;
		if(ipbuf[iplen-1]!=0x7E)
			return;
		DataFenBao(ipbuf+1,iplen-2,SIM1);
	}
}
/*******************************************************************************
* Function Name  : NewIPSendOP
* Description    : 设备组包   往后台发数据
* Input          :
* Return         :
*******************************************************************************/
void NewIPSendOP(u8 * hexbuf,u16 len,u8 linknum)
{
	u8   tempSendbuf[512];
	u16  wpoint;
	u8   tpbuf[20];
	u8   le;

	if((len>512)||(len == 0))
	{
		//UARTWrite((U8*)"leth_err",8,DEBUG_COM);//debugggg
		return;
	}

	if(Get_Gprs_DTR()==1)
	{
		GPRS_DTR_L;
		UARTWrite((u8*)"\r\nwake\r\n",8,0);
		SysTick_Delay_Ms(20);
	}
	Lowpowertick = 0;

	memcopy(tempSendbuf,hexbuf,len);
	wpoint = TransferSendData(tempSendbuf,len);  //转义...

	memcopy(tpbuf,(u8*)"AT+QISEND=1,",12);
	tpbuf[10]=linknum;
	le=INT2Sry(wpoint,tpbuf+12);

	UARTWrite(tpbuf,12+le,GPRS_COM);
	UARTWrite((u8*)"\r\n",2,GPRS_COM);
	Rx1Num = 0;
	UARTWrite(tpbuf,12+le,DEBUG_COM);
	UARTWrite((u8*)"\r\n",2,DEBUG_COM);
	le=100;
	if(linknum == TCPLINK)
	{
		Tcpsendflag = 1;
		tcpsendtick = 0;
	}

	//SendData2TestCOM(Rx1Buf,50,DEBUG_COM);
	while(le)
	{
		FEED_WTDG;
		SysTick_Delay_Ms(5);
		//UARTWrite(Rx1Buf,Rx1Num,DEBUG_COM);

		if(rcv_usart1_end_flag == 1)
		{
			if ((Rx1Buf[0]== 0x0d)&&(Rx1Buf[1]== 0x0a))
			{
				if(Rx1Buf[2]=='>')
				{
					UARTWriteData(tempSendbuf,wpoint,GPRS_COM);
					//UARTWrite(tempSendbuf,wpoint,GPRS_COM);
					SendData2TestCOM(tempSendbuf,wpoint,DEBUG_COM);

					le=200;
					if(linknum == TCPLINK)
						ATSend_Err_Cnt = 0;
					else if(linknum == UDPLINK)
						ATSend_Err_Cnt11 = 0;

					ATRE_Err_Cnt = 0;
					ATRE_Null_Cnt=0;
					break;
				}
			}
		}
		le--;
	}
	if(le!=200)
	{
		UARTWriteData(tempSendbuf,wpoint,GPRS_COM);
		//SendData2TestCOM(tempSendbuf,wpoint,DEBUG_COM);

		if(linknum == TCPLINK)
		{
			ATSend_Err_Cnt++;
		}
		else if(linknum == UDPLINK)
		{
			ATSend_Err_Cnt11++;
		}
			ATRE_Err_Cnt++;

		UARTWrite((u8*)"\r",1,GPRS_COM);
	}
}

//==================================================================//
void Module_ini_forGPRS(void)//模块连SERVER 步骤1.2.3.4  连后台
{
	if(MODULE_RESETed!='Y')
		return;

	if(gpsriniflag == 0)
		return;

	if(++GPRS_NET_TIPtick<120)
		return;
	GPRS_NET_TIPtick=0;
	if(GSMRegisted)
	{
		if(GPRSRegisted==0)
		{
			ATCmdFIFO((u8 *)"AT+CGREG?\r\0",ASK_CGREG);
			GPRS_NET_TIPtick=116;					//7s  20150413
			//GPRS_NET_TIPtick=250; 				 //5s  20150321
			return;
		}
		if(ConnectIndex == 2)
		{
			ATCmdFIFO((u8 *)"AT+QIDEACT\r\0",GPRS_DEACT2);
			GPRS_NET_TIPtick=96;
			GPRSReConnectFlag = 0X03;
			TCP_ErrorCnt=1;
			ConnectIndex = 3;
			GSMCHECKcnt = 160;			 //20s 20150420
			return;
		}
		if(ConnectIndex == 3)
		{
			ATCmdFIFO((u8 *)DNS_ADDR,DNS_IP);		//解析域名...
			GPRS_NET_TIPtick=100;
			TCP_ErrorCnt=1;
			ConnectIndex = 4;
			GSMCHECKcnt = 160;			 //20s 20150420
			gprsindex = 8;
			return;
		}

		switch(gprsindex)
		{
			case 0:
				ATCmdFIFO((u8 *)"AT+QIDEACT\r\0",GPRS_DEACT2);
				if(ConnectIndex == 0)
					GPRS_NET_TIPtick=116;
				else
					GPRS_NET_TIPtick=96;
				TCP_ErrorCnt=1;
				GSMCHECKcnt = 160;
				gprsindex = 1;
				break;
			case 1:
				ATCmdFIFO((u8 *)"AT+QISTATE\r\0",GPRS_QISTATE);
				GPRS_NET_TIPtick=116;
				TCP_ErrorCnt=1;
				GSMCHECKcnt = 160;
				gprsindex = 2;
				break;
			case 2:
			GPRS_NET_TIPtick=116;
			break;
			case 3:
				ATCmdFIFO((u8 *)"AT+QIREGAPP\r\0",GPRS_QIREFAPP);
				GPRS_NET_TIPtick=116;
				TCP_ErrorCnt=1;
				GSMCHECKcnt = 160;
				gprsindex = 4;
				break;
			case 4:
				ATCmdFIFO((u8 *)"AT+QISTATE\r\0",GPRS_QISTATE);
				GPRS_NET_TIPtick=116;
				TCP_ErrorCnt=1;
				GSMCHECKcnt = 160;
				gprsindex = 5;
				break;
			case 5:
				GPRS_NET_TIPtick=116;
				break;
			case 6:
				ATCmdFIFO((u8 *)"AT+QIACT\r\0",GPRS_QIACT);
				GPRS_NET_TIPtick=112;
				TCP_ErrorCnt=1;
				GSMCHECKcnt = 160;
				gprsindex = 7;
				break;
			case 7:
				ATCmdFIFO((u8 *)"AT+QISTATE\r\0",GPRS_QISTATE);
				GPRS_NET_TIPtick=116;
				TCP_ErrorCnt=1;
				GSMCHECKcnt = 160;
				gprsindex = 8;
				break;
			case 8:
				GPRS_NET_TIPtick=116;
				break;
			case 9:
				ATCmdFIFO((u8 *)"AT+QILOCIP\r\0",GPRS_QILOCIP);
				GPRS_NET_TIPtick=116;
				TCP_ErrorCnt=1;
				GSMCHECKcnt = 160;
				gprsindex = 10;
				break;
			case 10:
				ATCmdFIFO((u8 *)"AT+QISTATE\r\0",GPRS_QISTATE);
				GPRS_NET_TIPtick=116;
				TCP_ErrorCnt=1;
				GSMCHECKcnt = 160;
				gprsindex = 11;
				break;
			default:
				GPRS_NET_TIPtick=116;
				break;
		}
	}
	else
	{
		GPRS_NET_TIPtick=116;
		return;
	}
	if(TCPIP_been_OK)
	{
		return;
	}
	if(((GPRSReConnectFlag == 0X03)&&(gprsindex == 12)&&(ConnectIndex != 4))
	||((GPRSReConnectFlag == 0X03)&&(gprsindex == 12)&&(ConnectIndex == 4)&&(ipaddrlen)))
	{
		ConnectServer();
		GPRSReConnectFlag = 0;
	}
}
/*******************************************************************************
* Function Name  : SMSLockparameterdata
* Description    : 短信指令回复组包
* Input          :
* Return         :
*******************************************************************************/
u8 SMSLockparameterdata(u8* smsdata)
{
	//<ULOCK*N:19000000001*D:000.000.000.000,0000*T:000.000.000.000,0000*G:jt1.gghypt.net*A:CMNET>
	//<ULOCK*N:19000000001*D:058.214.242.018,6666*T:203.080.144.166,5088*G:server.siruide.net*A:CMNET>
	u8 buff[65],result=0,i,j;
	u8 tem[12];

	memcopy(smsdata,(u8*)"<ULOCK*N:",9);  //9
	result +=9;
	HEX2ASCII(DevParameters.snid,6,tem);     //12
	for(i=1;i<12;i++)
		smsdata[9+i-1] = tem[i];//舍弃第一位0
	result +=11;
	smsdata[result] = '*';result++;
	smsdata[result] = 'D';result++;
	smsdata[result] = ':';result++;

	memcopy(buff,TCP_SERVER,TCPCHARLEN);
	memcopy(smsdata+result,buff+19,15);
	result +=15;
	smsdata[result] = ',';result++;
	for(i = 0;i<TCPCHARLEN;i++)
	{
		if(buff[i] == '\r')
		{
			j = i;
		}
	}
	i = AT_Comma_Pos(buff,3);
	if(j>(i+2))
		memcopy(smsdata+result,buff+36,(j-i));
	result += (j-i);
	smsdata[result] = '*';result++;
	smsdata[result] = 'T';result++;
	smsdata[result] = ':';result++;

	memcopy(buff,TCP_SERVER2,TCPCHARLEN);
	memcopy(smsdata+result,buff+19,15);
	result +=15;
	smsdata[result] = ',';result++;
	for(i = 0;i<TCPCHARLEN;i++)
	{
		if(buff[i] == '\r')
		{
			j = i;
		}
	}
	i = AT_Comma_Pos(buff,3);
	if(j>(i+2))
	{
		memcopy(smsdata+result,buff+36,(j-i));
		result += (j-i);
	}
	smsdata[result] = '*';result++;
	smsdata[result] = 'G';result++;
	smsdata[result] = ':';result++;
	memcopy(buff,DNS_ADDR,str_len(DNS_ADDR)+1);	  //"AT+QIDNSGIP=\"server.siruide.net"\r\0"
	i = AT_Quota_Pos(buff,1);
	j = AT_Quota_Pos(buff,2)-1;
	if(j>i)
	{
		memcopy(smsdata+result,buff+13,(j-i));////////////////
		result += (j-i);///////////////////
	}
	smsdata[result] = '*';result++;
	smsdata[result] = 'A';result++;
	smsdata[result] = ':';result++;	  //"AT+QICSGP=1,\"CMNET"\r\0"
	memcopy(buff,APN_PARA,64);
	i = AT_Quota_Pos(buff,1);
	j = AT_Quota_Pos(buff,2)-1;
	if(j>i)
	{
		memcopy(smsdata+result,buff+13,(j-i));/////////////////
		result += (j-i);////////////////
	}
	smsdata[result] = '>';
	result++;
	return result;
}
/*******************************************************************************
* Function Name  : SMSDataOP
* Description    : 短信指令处理函数   接收-解析-发送
* Input          :
* Return         :
*******************************************************************************/
void SMSDataOP(u8 * dbuf,u8 dlen)
{
	u8 sms[120],hexpt,result;

		if((dbuf[0]=='<')&&(dbuf[dlen-3]=='>'))
		{
			UARTWrite((u8*)"SMSDATA_OK\r\n",12,DEBUG_COM);//debugggg
			//delay_ms(0);
			hexpt = dlen-2;
			if(hexpt>150)
				return;
			memcopy(sms,dbuf,hexpt);

			if(str_cmp(sms, "<ULOCK\0"))
			{
				result = SMSdataAnalysis(sms,hexpt);
				if(result == 0)
				{
					result = SMSLockparameterdata(SMS_SEND_BUF);
					SMSDataSend(SMS_SEND_BUF,result,SMS_in_Num);
				}
				else if(result == 1)
					SMSDataSend((u8*)"<Password Error!>",17,SMS_in_Num);
				else if(result == 2)
					SMSDataSend((u8*)"<SN Error!>",11,SMS_in_Num);
			}
			else if(str_cmp(sms, "<CXULOCK>\0"))
			{
				result = SMSLockparameterdata(SMS_SEND_BUF);
				SMSDataSend(SMS_SEND_BUF,result,SMS_in_Num);
			}
		}
}
/*******************************************************************************
* Function Name  : SMSdataAnalysis
* Description    : 短信指令处理函数   解析
* Input          :
* Return         :  0:设置 参数 ok；1：密码错误 ；2：锁号不匹配
*******************************************************************************/
u8 SMSdataAnalysis(u8* data,u8 len)
{
	u8 asteriskindex[6],colonindex[6],ascsid[11],tem[12],comma[6];
	u8 strbuff[70],apnbuff[24],username[10],password[8];
	u8 i,j,res,flag;
//<ULOCK*P:201712*N:19000000001*D:000.000.000.000,0000*T:000.000.000.000,0000*G:jt1.gghypt.net*A:CMNET>
	flag = 0;
	res = 0xFF;
	for(i=0;i<6;i++)
	{
		asteriskindex[i] = AT_Asterisk_Pos(data,i+1);//*
		colonindex[i] = AT_Colon_Pos(data,i+1);//:
	}

	for(j=0;j<6;j++)
	{
		switch(data[asteriskindex[j]])
		{
			//case 'p':
			case 'P':
				if(str_cmp(data+colonindex[0], SMSPASSWORD))
					flag = 1;
				else
				{
					flag = 0;
					res = 1;
				}
				break;
			case 'N':
			//case 'n':
				if((flag)&&(res!=1))
				{
					HEX2ASCII(DevParameters.snid,6,tem);
					for(i=0;i<11;i++)
						ascsid[i]=tem[i+1];	//去掉SN号前面的0
					if(memCpare(data+colonindex[1], ascsid,11))
						flag = 1;
					else
					{
						flag = 0;
						res = 2;
					}
				}
				break;
			case 'D':
			//case 'd':
				if(flag)
				{
						EEPROM_OP((u8*)TCP_SERVER,TCP_ADDR1,TCPCHARLEN,MODE_R);
						EEPROM_OP((u8*)UDP_SERVER,UDP_ADDR1,TCPCHARLEN,MODE_R);
						memcopy(TCP_SERVER+19,data+colonindex[2],15);//ip
						TCP_SERVER[34]='"';
						memcopy(UDP_SERVER+19,data+colonindex[2],15);//ip
						UDP_SERVER[34]='"';
						UARTWrite((u8*)"+IP1set\r\n",9,DEBUG_COM);//debugggggg
						if(asteriskindex[3]-asteriskindex[2] >18)
						{
							i = asteriskindex[3]-asteriskindex[2] - 19;
							memcopy(TCP_SERVER+36,data+colonindex[2]+16,i);//tcpport  2008
							UARTWrite((u8*)"+TCP1set\r\n",10,DEBUG_COM);//debugggggg
							TCP_SERVER[36+i]='\r';
							TCP_SERVER[36+i+1]='\0';
							res = 0;
						}
						EEPROM_OP((u8*)TCP_SERVER,TCP_ADDR1,TCPCHARLEN,MODE_W);
						EEPROM_OP((u8*)UDP_SERVER,UDP_ADDR1,TCPCHARLEN,MODE_W);
				}
				break;
			case 'T':
			//case 't':
				if(flag)
				{
						EEPROM_OP((u8*)TCP_SERVER2,TCP_ADDR2,TCPCHARLEN,MODE_R);
						EEPROM_OP((u8*)UDP_SERVER2,UDP_ADDR2,TCPCHARLEN,MODE_R);
						memcopy(TCP_SERVER2+19,data+colonindex[3],15);//ip
						TCP_SERVER2[34]='"';
						memcopy(UDP_SERVER2+19,data+colonindex[3],15);//ip
						UDP_SERVER2[34]='"';
						UARTWrite((u8*)"+IP2set\r\n",9,DEBUG_COM);//debugggggg
						if(asteriskindex[4]-asteriskindex[3] >18)
						{
							i = asteriskindex[4]-asteriskindex[3] - 19;
							memcopy(TCP_SERVER2+36,data+colonindex[3]+16,i);//tcpport  2008
							UARTWrite((u8*)"+TCP1set\r\n",10,DEBUG_COM);//debugggggg
							TCP_SERVER2[36+i]='\r';
							TCP_SERVER2[36+i+1]='\0';
							res = 0;
						}
						EEPROM_OP((u8*)TCP_SERVER2,TCP_ADDR2,TCPCHARLEN,MODE_W);
						EEPROM_OP((u8*)UDP_SERVER2,UDP_ADDR2,TCPCHARLEN,MODE_W);
				}
				break;
			case 'G':
			//case 'g':
				if(flag)
				{
					i = asteriskindex[5]-asteriskindex[4];
					if((i>2)&&(i <=30))
					{
						str_cpy(DNS_ADDR,(u8*)"AT+QIDNSGIP=\"\0");

						memcopy(DNS_ADDR+13,data+colonindex[4],(i-2));
						DNS_ADDR[13+i]='"';
						DNS_ADDR[14+i]='\r';
						DNS_ADDR[15+i]='\0';
						EEPROM_OP((u8*)DNS_ADDR,DNS_ADDR1,str_len(DNS_ADDR)+1,MODE_W);
						UARTWrite((u8*)"+DNSset\r\n",9,DEBUG_COM);//debuggggggggggggggggggggggg
						res = 0;
					}
				}
				break;
			case 'A':
			//case 'a':
				if(flag)
				{
					if(len > (colonindex[5]+1))
					{
						//AT+QICSGP=1,"CMNET","1234","567"
						for(i = 0 ;i<6;i++)
							comma[i] = 0;
						j = 0;
						apnbuff[0] = 0;
						username[0] = 0;
						password[0] = 0;
						EEPROM_OP((u8*)strbuff,APN_ADDR1,64,MODE_R);
						for(i = 0;i<64;i++)
						{
							if(strbuff[i] == 0x22)
							{
								comma[j] = i;
								j++;
							}
							if(strbuff[i] == '\r')
							{
								break;
							}
						}
						if((comma[1] - comma[0]) > 0)
						{
							apnbuff[0] = (comma[1] - comma[0]-1);
							memcopy(apnbuff+1,strbuff+comma[0]+1,apnbuff[0]);
						}
						if((comma[3] - comma[2]) > 0)
						{
							username[0] = (comma[3] - comma[2]-1);
							memcopy(username+1,strbuff+comma[2]+1,username[0]);
						}
						if((comma[5] - comma[4]) > 0)
						{
							password[0] = (comma[5] - comma[4]-1);
							memcopy(password+1,strbuff+comma[4]+1,password[0]);
						}
						j = len-colonindex[5]-1;
						memcopy(strbuff+13,data+colonindex[5],j);
						strbuff[13+j] = '"';
						if(username[0] == 0)
						{
							strbuff[13+j+1] = ',';
							strbuff[13+j+2] = '"';
							strbuff[13+j+3] = '"';
						}
						else
						{
							strbuff[13+j+1] = ',';
							strbuff[13+j+2] = '"';
							memcopy(strbuff+13+j+3,username+1,username[0]);
							strbuff[13+j+3+username[0]] = '"';
						}
						if(password[0] ==0)
						{
							strbuff[13+j+3+username[0]+1] = ',';
							strbuff[13+j+3+username[0]+2] = '"';
							strbuff[13+j+3+username[0]+3] = '"';
							strbuff[13+j+3+username[0]+4] = '\r';
							strbuff[13+j+3+username[0]+5] = '\0';
						}
						else
						{
							strbuff[13+j+3+username[0]+1] = ',';
							strbuff[13+j+3+username[0]+2] = '"';
							memcopy(strbuff+13+j+3+username[0]+3,password+1,password[0]);
							strbuff[13+j+3+username[0]+3+password[0]] = '"';
							strbuff[13+j+3+username[0]+3+password[0]+1] = '\r';
							strbuff[13+j+3+username[0]+3+password[0]+2] = '\0';
						}
						EEPROM_OP((u8*)strbuff,APN_ADDR1,64,MODE_W);
						res = 0;
					}
				}
				break;
			default:
				break;
		}
	}
	return res;
}
/*******************************************************************************
* Function Name  : SMSDataSend
* Description    : 让模块发短信，
* Input          :
* Return         :
*******************************************************************************/
void SMSDataSend(u8 * smsdata,u8 smslen,u8 * SMS_server)
{
	u8 tempSendbuf[29];
	u8 pt;
	u16 le;

	if(Get_Gprs_DTR()==1)
	{
		GPRS_DTR_L;
		UARTWrite((u8*)"\r\nwake\r\n",8,0);
		SysTick_Delay_Ms(20);
	}
	Lowpowertick = 0;
	memcopy(tempSendbuf,(u8*)"AT+CMGS=\"",9);pt=9;
	memcopy(tempSendbuf+pt,(u8*)SMS_server,str_len(SMS_server));pt+=str_len(SMS_server);
	memcopy(tempSendbuf+pt,(u8*)"\"\r",2);pt+=2;
	UARTWrite(tempSendbuf,pt,GPRS_COM);
	UARTWrite(tempSendbuf,pt,0);

	Rx1Num = 0;

	le=1000;
	while(le)
	{
		FEED_WTDG;
		SysTick_Delay_Ms(5);

		if(rcv_usart1_end_flag == 1)
		{
			//if ((Rx1Buf[0]== 0x0d)&&(Rx1Buf[1]== 0x0a))
			{
				if((Rx1Buf[0]=='>')||(Rx1Buf[1]== '>')||(Rx1Buf[2]== '>'))
				{
					UARTWriteData(smsdata,smslen,GPRS_COM);
					Usart_SendByte(USART1,0x1A);
					UARTWrite(smsdata,smslen+1,0);//打印短信回发的内容
					le=2000;ATRE_Null_Cnt=0;
					break;
				}
			}
		}
		le--;
	}
	if(le!=2000)
	{
		memcopy(SMSDATAbuf+1,smsdata,smslen);
		SMSDATAbuf[0]=smslen;
	}
}

/*******************************************************************************
* Function Name  : LockRecUP
* Description    : LockRecUP
* Input          :
* Return         :
*******************************************************************************/
void LockRecUP(u8 *seqnobuf)
{
  	u8	var,cp;
	u8 tagetdata[120];
	u8 tagindexandlen[2];
	u8 tagfuccode[2];


  	for(var=0;var<MAXLockRECVOL;var++)
	{
		EEPROM_OP(tagindexandlen,((u32)(TAGRECORDBUFF)+(u32)var*HVOL), 2, MODE_R);
		if((tagindexandlen[0]==0)||(tagindexandlen[0]>(u8)(DevParameters.GprsRetrycount)))
		{
			continue;
		}
		cp = tagindexandlen[1] -1;
		EEPROM_OP(tagfuccode,((u32)(TAGRECORDBUFF)+(u32)var*HVOL+cp), 2, MODE_R);
		if(!memCpare(tagfuccode,(u8*)seqnobuf,2))
		{
			continue;
		}
		tagindexandlen[0]++;
		if(tagindexandlen[0] >=(u8)(DevParameters.GprsRetrycount))
		{
			tagindexandlen[0] = 0;
		}
		EEPROM_OP((u8 *)&tagindexandlen[0],((u32)(TAGRECORDBUFF)+(u32)var*HVOL), 1, MODE_W);

		EEPROM_OP(tagetdata,((u32)(TAGRECORDBUFF)+(u32)var*HVOL+2), tagindexandlen[1], MODE_R);

		SendData2GPRSCOM(tagetdata,tagindexandlen[1]);
		UARTWrite((u8*)"Res\r\n",5,DEBUG_COM);
	}
}

/*******************************************************************************
* Function Name  : LockRecReUP
* Description    : //数据重传//
* Input          :
* Return         :
*******************************************************************************/
void LockRecReUP(void)
{
	u8 tmp,cp;
	u16 seq;
	u8 tagindexandlen[2];

	for(tmp=TrIndex;tmp<MAXLockRECVOL;tmp++)
	{
		SysTick_Delay_Ms(5);

		EEPROM_OP(tagindexandlen,((u32)(TAGRECORDBUFF)+(u32)tmp*HVOL), 2, MODE_R);
		if((tagindexandlen[0]==0)||(tagindexandlen[0]>(u8)(DevParameters.GprsRetrycount)))
		{
			continue;
		}
		cp = tagindexandlen[1] -1;
		EEPROM_OP((u8*)&seq,((u32)(TAGRECORDBUFF)+(u32)tmp*HVOL+cp), 2, MODE_R);
		LockRecUP((u8*)&seq);
		TrIndex=tmp+1;
		if(TrIndex>=MAXLockRECVOL)
		{
			TrIndex=0;
		}
		break;
	}
	if(tmp == MAXLockRECVOL)
	{
		for(tmp=0;tmp<TrIndex;tmp++)
		{
			SysTick_Delay_Ms(5);

			EEPROM_OP(tagindexandlen,((u32)(TAGRECORDBUFF)+(u32)tmp*HVOL), 2, MODE_R);
			if((tagindexandlen[0]==0)||(tagindexandlen[0]>(u8)(DevParameters.GprsRetrycount)))
			{
				continue;
			}
			cp = tagindexandlen[1] -1;
			EEPROM_OP((u8*)&seq,((u32)(TAGRECORDBUFF)+(u32)tmp*HVOL+cp), 2, MODE_R);
			LockRecUP((u8*)&seq);
			TrIndex=tmp+1;
			if(TrIndex>=MAXLockRECVOL)
			{
				TrIndex=0;
			}
			break;
		}
	}
}

void NewLockRecReUP(void)//新协议  重发
{
	if(AuthorizeProtocol.retrytick >0)
	{
		if((TCPIP_been_OK&0X01)==0X01)
		{
			NewIPSendOP(AuthorizeProtocol.authprotodata,AuthorizeProtocol.datalen,TCPLINK);
		}
		else if((TCPIP_been_OK&0X02)==0X02)
		{
			NewIPSendOP(AuthorizeProtocol.authprotodata,AuthorizeProtocol.datalen,UDPLINK);
		}

		AuthorizeProtocol.retrytick--;

		UARTWrite((u8*)"Res\r\n",5,DEBUG_COM);
	}

}

/*******************************************************************************
* Function Name  : ClrLockOPRec
* Description    : 清除EE内容
* Input          :
* Return         :
*******************************************************************************/
u8  ClrLockOPRec(u8 mode)
{
	u8 tmp;

	SysTick_Delay_Ms(5);

	if(mode == cGPRS_TRLIST)
	{
		//清除GPRS记录发送列表//
		for(tmp=0;tmp<MAXLockRECVOL;tmp++)
		{
		//*(TagRecBuf+tmp*HVOL)=0;
		}
	}
	return 1;
}
/*******************************************************************************
* Function Name  : SendData2GPRSCOM
* Description    : 发数据
* Input          :
* Return         :
*******************************************************************************/
u8 SendData2GPRSCOM(u8 *Save_Data,u8 lenth)
{
	if((TCPIP_been_OK&0X01)==0X01)
	{
		NewIPSendOP(Save_Data,lenth,TCPLINK);
		return 1;
	}
	else if((TCPIP_been_OK&0X02)==0X02)
	{
		NewIPSendOP(Save_Data,lenth,UDPLINK);
		return 1;
	}
	return 0;

}

/*******************************************************************************
* Function Name  : GetSMSNUM
* Description    :获取手机号码
* Input          :
* Return         :
*******************************************************************************/
void GetSMSNUM(u8 * buf)
{
	//: "+8613798183208",,"2013/03/23 11:51:18+32"
	//: "8613798183208",,"2013/03/29 12:03:03+32"
	//: "18651565633",,"2015/12/28 12:29:32+32"
	//: "106550010646",,"2015/12/29 09:44:46+32"
	//: "+861064690471365",,"2016/01/04 14:48:17+32"
	//: "18552193518",,"2018/01/28 19:12:45+32"
	extern u8  SMS_in_Num[20];
	if((buf[3]=='+')&&(buf[4]=='8')&&(buf[5]=='6')&&(buf[17]=='"')&&(buf[18]==','))
	{
		memcopy(SMS_in_Num,buf+6,11);
		SMS_in_Num[11]=0;
		UARTWrite((u8*)SMS_in_Num,11,DEBUG_COM);//debugggg
		return;
	}
	if((buf[3]=='8')&&(buf[4]=='6')&&(buf[16]=='"')&&(buf[17]==','))
	{
		memcopy(SMS_in_Num,buf+5,11);
		SMS_in_Num[11]=0;
		UARTWrite((u8*)SMS_in_Num,11,DEBUG_COM);//debugggg
		return;
	}
	if((buf[3]=='1')&&(buf[14]=='"')&&(buf[15]==','))
	{
		memcopy(SMS_in_Num,buf+3,11);
		SMS_in_Num[11]=0;
		UARTWrite((u8*)SMS_in_Num,11,DEBUG_COM);//debugggg
		return;

	}
	if((buf[3]=='1')&&(buf[15]=='"')&&(buf[16]==','))
	{
		memcopy(SMS_in_Num,buf+3,12);
		SMS_in_Num[12]=0;
		UARTWrite((u8*)SMS_in_Num,12,DEBUG_COM);//debugggg
		return;

	}
	if((buf[3]=='+')&&(buf[4]=='8')&&(buf[5]=='6')&&(buf[19]=='"')&&(buf[20]==','))
	{
		memcopy(SMS_in_Num,buf+6,13);
		SMS_in_Num[13]=0;
		UARTWrite((u8*)SMS_in_Num,13,DEBUG_COM);//debugggg
		return;

	}
	if((buf[3]=='8')&&(buf[4]=='6')&&(buf[18]=='"')&&(buf[19]==','))
	{
		memcopy(SMS_in_Num,buf+5,13);
		SMS_in_Num[13]=0;
		UARTWrite((u8*)SMS_in_Num,13,DEBUG_COM);//debugggg
		return;

	}
	if((buf[3]=='1')&&(buf[15]=='"')&&(buf[16]==','))
	{
		memcopy(SMS_in_Num,buf+3,12);
		SMS_in_Num[12]=0;
		UARTWrite((u8*)SMS_in_Num,12,DEBUG_COM);//debugggg
		return;

	}
  UARTWrite((u8*)buf,18,DEBUG_COM);//debugggg


}
/*******************************************************************************
* Function Name  : GetSMSData
* Description    :获取短信信息
* Input          :
* Return         :
*******************************************************************************/
void GetSMSData(u8 * dbuf,u8 * buf,u8 len)
{
//004600450038003200420042003100320033003400350036003700380039003000460046
	u8 i;
	if(len%4 == 0)
	{
		for(i = 0;i< len/4;i++)
		{
			if((*(buf+2+i*4)>=0x30)&&(*(buf+2+i*4)<=0x39))
				*(buf+2+i*4) = *(buf+2+i*4)- 0x30;
			else if((*(buf+2+i*4)>=0x41)&&(*(buf+2+i*4)<=0x5A))
				*(buf+2+i*4) = *(buf+2+i*4) - 0x37;
			else if((*(buf+2+i*4)>=0x61)&&(*(buf+2+i*4)<=0x7A))
				*(buf+2+i*4) = *(buf+2+i*4) - 0x57;
			if((*(buf+3+i*4)>=0x30)&&(*(buf+3+i*4)<=0x39))
				*(buf+3+i*4) = *(buf+3+i*4)- 0x30;
			else if((*(buf+3+i*4)>=0x41)&&(*(buf+3+i*4)<=0x5A))
				*(buf+3+i*4) = *(buf+3+i*4) - 0x37;
			else if((*(buf+3+i*4)>=0x61)&&(*(buf+3+i*4)<=0x7A))
				*(buf+3+i*4) = *(buf+3+i*4) - 0x57;
			*(dbuf+i) = *(buf+2+i*4)*16+*(buf+3+i*4);

		}
	}
}

void Check_GPRS_Data(u8 *Buff,u16 dleth)
{
	u16 i = 0;
	u16 lastpt = 0,datalen = 0;
	u8	j = 0,k = 0;

	//没有新数据//
	if(dleth == 0)
		return;

	datalen = dleth;
	UARTWrite(Buff,dleth,DEBUG_COM);

	NVIC_DisableIRQ(USART1_IRQn);/* 关串口接收中断 */
	do
	{
		FEED_WTDG;
		//先将所有数据过滤下，找到头再处理//
		for(i = 0;i<datalen;i++)
		{
			if(((*(Buff+i) == 0x0d)&&(*(Buff+(i+1)) == 0x0a))||																	//\r\n
				((*(Buff+i) == 'N')&&(*(Buff+(i+1)) == 'O')) ||																		//NO
			((*(Buff+i) == 'I')&&(*(Buff+(i+1)) == 'P')&&(*(Buff+(i+2)) == 'D'))||							//IPD
			((*(Buff+i) == '+')&&(*(Buff+(i+1)) == 'C')&&(*(Buff+(i+2)) == 'L'))||							//+CL
			((*(Buff+i) == 'R')&&(*(Buff+(i+1)) == 'I'))||																				//RI
			((*(Buff+i) == 'N')&&(*(Buff+(i+1)) == 'G')) )																				//NG
			{
				lastpt = i;
				break;
			}
			else
			{
				if(((Buff[i] == '+')&&(Buff[i+1] =='C')&&(Buff[i+2] =='M')&&(Buff[i+3] =='T'))||	//+CMT
					((Buff[i] == '+')&&(Buff[i+1] =='C')&&(Buff[i+2] =='P')&&(Buff[i+3] =='B'))||		//+CPB
					((*(Buff+i)=='M')&&(*(Buff+(i+1))=='O'))    )	 																		//MO
				{
					lastpt = i;
					break;
				}
			}
		}

		if(datalen >lastpt)
		{
			for(i = 0;i<(datalen - lastpt);i++)
			{
				*(Buff+i) = *(Buff+(lastpt + i));
			}
		}
		datalen = i;
		if((*(Buff+0) == 0x0d)&&(*(Buff+1) == 0x0a)&&(*(Buff+2) == '+')&&(*(Buff+3) == 'R') &&(*(Buff+10) == ':')  )//0D0A+RECEIVE: 1, 70D0AFE.....FF
		{
			//0D 0A 2B 52 45 43 45 49 56 45 3A 20 31 2C 20 32 37 0D 0A FE 21 10 00 19 31 32 2F 30 36 2F 32 30 2C 31 30 3A 32 34 3A 35 36 2B 30 38 FA FF
			for(i = 10;i<datalen;i++)
			{
				if((*(Buff+i) == ',')&&(*(Buff+(i+1)) == 0x20))
				{
					j=RCVAtoI(Buff+i+2);   // 数据长度	u8

				}
				if((*(Buff+i) == 0x0d)&&(*(Buff+(i+1)) == 0x0a))
				{
					break;
				}
			}
			lastpt=j+i+1;

			if( (j+3)<=IPBUF_SIZE )
			{
				for(k = 0;k<IP_RECBUF_CNT;k++)
				{
					if(IP_RCV_Buff[k][0]!='R')
						break;
				}
				if(k >= IP_RECBUF_CNT)
				{
					k = 0;
				}
				IP_RCV_Buff[k][0]='R';
				IP_RCV_Buff[k][1]=j;
				memcopy((u8*)&IP_RCV_Buff[k][2],(u8*)Buff+i+2,j);
			}
		}
		else if((*(Buff+0) == 0x0d)&&(*(Buff+1) == 0x0a))
		{
			if(*(Buff+2) == '>')
			{
				lastpt = 3;
			}
			else if ((Buff[2] == '+')&&(Buff[3] =='C')&&(Buff[4] =='M')&&(Buff[5] =='T'))
			{
				for(i = 5;i<datalen;i++)
				{
					if((*(Buff+i) == 0x0d)&&(*(Buff+(i+1)) == 0x0a))
					{
						break;
					}
				}
				for(k = i+2;k<datalen;k++)
				{
					if((*(Buff+k) == 0x0d)&&(*(Buff+(k+1)) == 0x0a))
					{
						lastpt = (k+1);
						break;
					}
				}
				if(((lastpt-i-1)<180)&& (((Buff[i+2]=='F')&&(Buff[i+3]=='E'))||((Buff[i+2]=='*')&&(Buff[i+3]=='#'))) )
				{
					GetSMSNUM(Buff+6);
					for(k = 0;k<SMS_RECBUF_CNT;k++)
					{
						if(SMS_RCV_Buff[k][0] != 'R')
						{
							break;
						}
					}
					if(k >= SMS_RECBUF_CNT)
					{
						k = 0;
					}
					for(j =i+2;j<(lastpt+1);j++)
						SMS_RCV_Buff[k][j-i] = *(Buff+j);
					SMS_RCV_Buff[k][0] = 'R';
					SMS_RCV_Buff[k][1] = (lastpt-i-1);
					//SendData2TestCOM((u8*)&SMS_RCV_Buff[k][2],SMS_RCV_Buff[k][1],0);
					printf("TTL*%s#\r\n",(u8*)&SMS_RCV_Buff[k][2]);
				}
				if(((lastpt-i-1)<180)&&((Buff[i+2]=='0')&&(Buff[i+3]=='0')&&(Buff[i+4]=='4')&&(Buff[i+5]=='6')))
				{
					GetSMSNUM(Buff+6);
					if((lastpt-i-1-2)>0)
					{
						for(k = 0;k<SMS_RECBUF_CNT;k++)
						{
							if(SMS_RCV_Buff[k][0] != 'R')
							{
								break;
							}
						}
						if(k >= SMS_RECBUF_CNT)
						{
							k = 0;
						}

						GetSMSData((u8 *)&SMS_RCV_Buff[k][2],(Buff+i+2),(lastpt-i-1-2));

						SMS_RCV_Buff[k][0] = 'R';
						SMS_RCV_Buff[k][1] = ((lastpt-i-1-2)/4)+2;
						//SendData2TestCOM((u8*)&SMS_RCV_Buff[k][2],SMS_RCV_Buff[k][1],0);
						printf("TTL*%s#\r\n",(u8*)&SMS_RCV_Buff[k][2]);
					}
				}
				if(((lastpt-i-1)<180)&&(Buff[i+2]=='<'))
				{
					GetSMSNUM(Buff+6);
					for(k = 0;k<SMS_RECBUF_CNT;k++)
					{
						if(SMS_RCV_Buff[k][0] != 'R')
						{
							break;
						}
					}
					if(k >= SMS_RECBUF_CNT)
					{
						k = 0;
					}
					for(j =i+2;j<(lastpt+1);j++)
						SMS_RCV_Buff[k][j-i] = *(Buff+j);
					SMS_RCV_Buff[k][0] = 'R';
					SMS_RCV_Buff[k][1] = (lastpt-i-1);
					//SendData2TestCOM((u8*)&SMS_RCV_Buff[k][2],SMS_RCV_Buff[k][1],0);
					printf("TTL*%s#\r\n",(u8*)&SMS_RCV_Buff[k][2]);
				}
			}
			else
			{
				j = 0;
				if((*(Buff+2) == 0x0d)&&(*(Buff+3) == 0x0a))
				{
					for(i = 4;i<datalen;i++)
					{
						if((*(Buff+i) == 0x0d)&&(*(Buff+(i+1)) == 0x0a))
						{
							//AT包的头和尾都找到了//
							lastpt = (i+1);break;
						}
					}
				}
				else
				{
					for(i = 2;i<datalen;i++)
					{
						if((*(Buff+i) == 0x0d)&&(*(Buff+(i+1)) == 0x0a))
						{
							//AT包的头和尾都找到了//
							lastpt = (i+1);break;
						}
					}
					if(*(Buff+2) == 0x0d)
					{
						lastpt = 1;
					}
				}

				if(lastpt>3)
				{
					if((lastpt+5)<=AT_INBUF_SIZE)
					{
						for(k = 0;k<AT_INBUF_CNT;k++)
						{
							if(AT_RCV_Buff[k][0] != 'R')
								break;
						}
						if(k >= AT_INBUF_CNT)
						{
							k = 0;
						}
						AT_RCV_Buff[k][2] = 0x0d;
						AT_RCV_Buff[k][3] = 0x0a;
						for(i = 2;i<(lastpt + 1);i++)
							AT_RCV_Buff[k][i+2] = *(Buff+i);

						AT_RCV_Buff[k][i+2] = 0;
						AT_RCV_Buff[k][0] = 'R';
						AT_RCV_Buff[k][1] =(lastpt + 1);
//SendData2TestCOM((u8*)&AT_RCV_Buff[k][2],AT_RCV_Buff[k][1],0);
					}
				}
			}
		}
		else if(((*(Buff+0) == '+')&&(*(Buff+1) == 'C')&&(*(Buff+2) == 'L'))||((*(Buff+0) == 'N')&&(*(Buff+1) == 'O')))//+CLIP: "",128//NO CARRIER////+ CCLK: YY/MM/DD,hh:mm:ss<+zz> 0D0A ((*(Buff+0) == '+')&&(*(Buff+1) == 'C')&&(*(Buff+2) == 'C'))
		{
			//UARTWrite((u8 *)"CALL_IN\r\n",9,DEBUG_COM);//debugggg
			for(i = 3;i<datalen;i++)
			{
				if((*(Buff+i) == 0x0d)&&(*(Buff+(i+1)) == 0x0a))
				{
					//clip包的头和尾都找到了//
					lastpt = (i+1);break;
				}
			}
			if((lastpt+5)<=AT_INBUF_SIZE)
			{
				for(k = 0;k<AT_INBUF_CNT;k++)
				{
					if(AT_RCV_Buff[k][0] != 'R')
						break;
				}

				if(k >= AT_INBUF_CNT)
				{
					k = 0;
				}
				AT_RCV_Buff[k][2]=0x0d;
				AT_RCV_Buff[k][3]=0x0A;
				for(i = 0;i<(lastpt + 1);i++)
				{
					AT_RCV_Buff[k][i+4]= *(Buff+i);
				}
				AT_RCV_Buff[k][i+4] = 0;
				AT_RCV_Buff[k][0] = 'R';
				AT_RCV_Buff[k][1] =(lastpt + 1);
			}
		}
		else if((*(Buff+0) == 'R')&&(*(Buff+1) == 'I'))//RING0D0A
		{
			for(i = 2;i<datalen;i++)
			{
				if((*(Buff+i) == 0x0d)&&(*(Buff+(i+1)) == 0x0a))
				{
					//clip包的头和尾都找到了//
					lastpt = (i+1);break;
				}
			}
		}
		else if ((Buff[0] == '+')&&(Buff[1] =='C')&&(Buff[2] =='M')&&(Buff[3] =='T'))//+cmt: 138...0D0Attttt0D0A//
		{
			//UARTWrite((u8 *)"SMS DATA\r\n",10,DEBUG_COM);//debugggg
			//+CMT: "8613560152253",,"09/05/15,09:18:10+32",145,4,0,8,"8613800755500",145,22
			//003300360038005B53D181EA00310033003990AE7BB1
			for(i = 5;i<datalen;i++)
			{
				if((*(Buff+i) == 0x0d)&&(*(Buff+(i+1)) == 0x0a))
				{
					break;
				}
			}
			for(k = i+2;k<datalen;k++)
			{
				if((*(Buff+k) == 0x0d)&&(*(Buff+(k+1)) == 0x0a))
				{
					lastpt = (k+1);
					break;
				}
			}
			//+CMT: "+8618680536752",,"2012/08/07 09:40:28+32"
			//FE31170308013117F9FF
			if(memCpare(Buff+i+2,(u8*)"FE31170308013117F9FF",6))
			{
				SysTick_Delay_Ms(1);
				IPset(0x30,&k,0);
				while(1);
			}
			GetSMSNUM(Buff+4);
			if(((lastpt-i-1)<180)&&((Buff[i+2]=='F')&&(Buff[i+3]=='E')))
			{
				for(k = 0;k<SMS_RECBUF_CNT;k++)
				{
					if(SMS_RCV_Buff[k][0] != 'R')
					{
						break;
					}
				}
				if(k >= SMS_RECBUF_CNT)
				{
					k = 0;
				}

				for(j =i+2;j<(lastpt+1);j++)
				SMS_RCV_Buff[k][j-i] = *(Buff+j);

				SMS_RCV_Buff[k][j-i] = 0;
				SMS_RCV_Buff[k][0] = 'R';
				SMS_RCV_Buff[k][1] = (lastpt-i-1);
				//SendData2TestCOM((u8*)&SMS_RCV_Buff[k][2],SMS_RCV_Buff[k][1],0);
				printf("TTL*%s#\r\n",(u8*)&SMS_RCV_Buff[k][2]);
			}
			else if(((lastpt-i-1)<180)&&((Buff[i+2]=='0')&&(Buff[i+3]=='0')&&(Buff[i+4]=='4')&&(Buff[i+5]=='6')))
			{
				if((lastpt-i-1-2)>0)
				{
					for(k = 0;k<SMS_RECBUF_CNT;k++)
					{
						if(SMS_RCV_Buff[k][0] != 'R')
						{
							break;
						}
					}
					if(k >= SMS_RECBUF_CNT)
					{
						k = 0;
					}

					GetSMSData((u8 *)&SMS_RCV_Buff[k][2],(Buff+i+2),(lastpt-i-1-2));

					SMS_RCV_Buff[k][0] = 'R';
					SMS_RCV_Buff[k][1] = ((lastpt-i-1-2)/4)+2;
					//SendData2TestCOM((u8*)&SMS_RCV_Buff[k][2],SMS_RCV_Buff[k][1],0);
					printf("TTL*%s#\r\n",(u8*)&SMS_RCV_Buff[k][2]);
				}
			}
			if(((lastpt-i-1)<180)&&(Buff[i+2]=='<'))
			{
				for(k = 0;k<SMS_RECBUF_CNT;k++)
				{
					if(SMS_RCV_Buff[k][0] != 'R')
					{
						break;
					}
				}
				if(k >= SMS_RECBUF_CNT)
				{
					k = 0;
				}

				for(j =i+2;j<(lastpt+1);j++)
				SMS_RCV_Buff[k][j-i] = *(Buff+j);

				SMS_RCV_Buff[k][j-i] = 0;
				SMS_RCV_Buff[k][0] = 'R';
				SMS_RCV_Buff[k][1] = (lastpt-i-1);
				//SendData2TestCOM((u8*)&SMS_RCV_Buff[k][2],SMS_RCV_Buff[k][1],0);
				printf("TTL*%s#\r\n",(u8*)&SMS_RCV_Buff[k][2]);
			}
		}
		//-------------------------------------//
		if(datalen> (lastpt+1) )
		{
			for(i = 0;i<(datalen-(lastpt+1));i++)
			{
				*(Buff+i) = *(Buff+((lastpt + 1) + i));
			}
			datalen = i;

			if(datalen<3)
			{
				datalen=0;
			}
		}
		else
		{
			datalen=0;
		}

		if(datalen > RX1_BUF_SIZE)
			break;

	}
	while(datalen!=0);

	memset(Rx1Buf,0x00,RX1_BUF_SIZE);//清空Rx1Buf

	Rx1Num = 0;

	NVIC_EnableIRQ(USART1_IRQn);/* 开串口接收中断 */
}

