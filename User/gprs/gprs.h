#ifndef __GPRS_H
#define __GPRS_H

#include 	"config.h"

struct bcInfo
{
  u8 Gps[36];
  u8 CarVol;
};

#define MAXLockRECVOL 20
#define RETRAN_TIMES 20
#define HVOL 120

#define SMSPASSWORD "201712\0"

#define  SIM1  '6'
#define  SIM2  '7'
#define  SMS1  '8'
#define  SMS2  '9'

#define AXKQ  0x00 //0X00AA

//开关机//
#define SOFT_PWR_OFF 0X01
#define SOFT_RESET   0X02
#define HD_PWR_ON  0X03
#define HD_PWR_OFF 0X04
#define EM_PWR_OFF 0X05
#define HD_PWR_OFF2 0X06
#define EM_PWR_OFF2 0X07
//模块状态:
#define LOWVDD_PWR_OFF  8
#define NORMAL_PWR_OFF  9
#define PWR_IN          10
#define IPINI           11
#define IPSTART         12
#define IPCONFIG        13
#define IPIND           14
#define IPGACT          15
#define IPSTSUS         16
#define TCPCONNECTING   17
#define UDPCONNECTING   18
#define IPCLOSE         19
#define TCPIP_ONLINE    20
#define GPRS_DEACT      21
#define GPRS_SET        22
#define GPRS_APN        22
#define SMS_SET         23
#define ASK_CSQ         24
#define AT_RE_OK        25
#define AT_RE_ERR       26
#define ASK_CREG        27
#define ASK_CGREG       28
#define GPRS_DEACT2     29
#define ASK_CBC         30
#define M35_KEEPOFF     31
#define AT_RE_NULL      32
#define AT_INI          33
#define CHECK_PIN       34
#define GPRS_QIREFAPP   35
#define GPRS_QIACT      36
#define GPRS_QILOCIP    37
#define  GPRS_QISTATE    38  

#define	IDLE_STATUS 0
#define	BUSY_STATUS 1
#define	AT_ID '&'
#define	SMS_ID 1
#define	GPRS_ID '$'
#define	TCP_DATA_ID 7
#define	UDP_DATA_ID 8
#define	NO_ID	0xFF

#define SET_CMEE  100
#define SET_CREG  101
#define SET_CGREG 102
#define SET_CLIP  103
#define ASK_CSCA  105
#define SET_STORE 106
#define SET_SLEEP 107
#define SMS_TIP_SET 108
#define SET_FUNC  109
#define TCPIP_OPEN  110
#define TCP_CLOSE 111
#define UDP_CLOSE 125
#define EXIT_TCPIP 112
#define EN_IOM   113
#define SET_CPBS 115
#define SET_PIN0 116
#define SET_PIN3 117
#define SET_PIN1 118
#define SET_PIN2 119
#define SET_PIN5 120
#define SET_PIN6 121
#define IN_PIN   122
#define ASK_PIN  123
#define VOICE_AAA   126
#define VOICE_DIAL  127
#define VOICE_END   128
#define VOICE_SET   129
#define ASK_TCPIP   130
#define SET_ATE   131
#define SET_CCLK  132
#define RM_DOP    133
#define CPBR     135
#define QSCLK_SET  136
#define DNS_IP  137


#define TCP_SEND '0'  
#define UDP_SEND '1'  
#define	TCPLINK '0'
#define	UDPLINK '1'
#define	SMSLINK '2'

//AT//
#define	AT_CMD_CNT	15
#define	AT_INBUF_CNT	10
#define	AT_INBUF_SIZE	80

#define	IP_RECBUF_CNT	3//7
#define	IPBUF_SIZE 255//128//20180301

//--------sms------
//SMS//
#define	SMSAT_MAXCNT	5
#define	SMS_RECBUF_CNT	2

#define	GSM_CHECK_PERIOD	240	//1分钟//

#define GPRS_DTR_PORT	GPIOB
#define GPRS_DTR_PIN		GPIO_Pin_12
#define GPRS_ON_PORT		GPIOB
#define GPRS_ON_PIN		GPIO_Pin_13

#define GPRS_DTR_H 		GPIO_SetBits(GPRS_DTR_PORT,GPRS_DTR_PIN);
#define GPRS_DTR_L 		GPIO_ResetBits(GPRS_DTR_PORT,GPRS_DTR_PIN);

#define GPRS_ON_H 			GPIO_ResetBits(GPRS_ON_PORT,GPRS_ON_PIN);
#define GPRS_ON_L 			GPIO_SetBits(GPRS_ON_PORT,GPRS_ON_PIN);

#define GPRS_POWER_OFF {GPIO_SetBits(GPRS_ON_PORT,GPRS_ON_PIN);\
												SysTick_Delay_Ms(1000);\
												GPIO_ResetBits(GPRS_ON_PORT,GPRS_ON_PIN);}

#define	RX1_BUF_SIZE   500//260//360 UART1_INBUF_SIZE
#define	RX2_BUF_SIZE   200//gps一次输出143个字节

extern u8 TrIndex;
extern u32 HandTick;
extern u8 GPRSReConnectFlag;
extern u8 MODULE_RESETed;
extern u16 MODULE_RESETed_Cnt;
extern u8 AT_CMD_list[AT_CMD_CNT][115];//15*115
extern u8 IP_RCV_Buff[IP_RECBUF_CNT][IPBUF_SIZE];
extern u8 AT_RCV_Buff[AT_INBUF_CNT][AT_INBUF_SIZE];	//接收到的AT数据的缓存//10*80
extern u8 SMS_RCV_Buff[SMS_RECBUF_CNT][200];
extern u8 CSQValue;
extern u8 SysTicker;
												
void GPRS_POWER_ON(void);
u8 Get_Gprs_DTR(void);
void NetModuleBaseIni(void);
void SMSIni(void);
void GPRSIni(void);
void ConnectServer(void);
void DisConnectServer(void);
void NetModulePowerOP(u8 mode);
u8 SearchIPchar(u8 *paras,u8 *IPaddress ,u8 len);
u8 AT_Quota_Pos(u8 *buf,u8 cx);
u8 AT_Asterisk_Pos(u8 *buf,u8 cx);
u8 AT_Comma_Pos(u8 *buf,u8 cx);
u8 AT_Colon_Pos(u8 *buf,u8 cx);
void NewLockRecReUP(void);
void ATResponseOP(u8 *paras);
void ATCmdFIFO(u8 * cmdbuf, u8	cmdtype);
u8 ATCmdSend(u8 taskid);
u8 ATCmdListPushUp(void);
u8 ATcmdEXE(u8 *atstr,u8 type);
void ModuleFlagsIni(void);
void NETFlagsIni(void);
void Module_TimerHandler(void);
void AT_TASK(void);
void GSMMCheck(void);
void IPGetOP(u8 * ipbuf,u8 iplen);
void Module_ini_forGPRS(void);   
void NewIPSendOP(u8 * hexbuf,u16 len,u8 linknum);
void GPRSMBRini(u8 com);
void SMSDataOP(u8 * dbuf,u8 dlen);
void SMSDataSend(u8 * smsdata,u8 smslen,u8 * SMS_server);
void SendSMS2R(void);
void LockRecUP(u8 *seqnobuf);
void LockRecReUP(void);
u8 ClrLockOPRec(u8 mode);
u8 SendData2GPRSCOM(u8 *Save_Data,u8 lenth);
void CenterDongZuoJG_UP(u16 cmd,u8 * buf,u8 type,u8 op);
void AT_DirectOP(u8 *atbuf,u8 leth);
void GetSMSNUM(u8 * buf);
void GetSMSData(u8 * dbuf,u8 * buf,u8 len);
void Check_GPRS_Data(u8 *Buff,u16 dleth);
u8 SMSdataAnalysis(u8* data,u8 len);
u8 SMSLockparameterdata(u8* smsdata);
#endif //__GPRS_H

