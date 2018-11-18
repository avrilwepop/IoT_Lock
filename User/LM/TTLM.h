#ifndef __TTLM_H__
#define __TTLM_H__

#include 	"config.h"

#define MODE_W   1
#define MODE_R	 2
#define MODE_ALARM   5

#define WORDMS ((unsigned int)(LockTicker&0x0000ffff))
#define GetTimer ((WORDMS>=StartTime)?(WORDMS-StartTime):(65535-StartTime+WORDMS))
#define NewGetTimer ((WORDMS>=NewStartTime)?(WORDMS-NewStartTime):(65535-NewStartTime+WORDMS))

#define RECORDDATALEN  44
#define NEWRECORDDATALEN 8

//证书预置初始化0x57，响应0x58
//证书预置指令 0x59，响应0x5A

#define CERTIFI_INIT      0x57
#define CERTIFI_INIT_BACK 0x58

#define CERTIFI_ISSU       0x59
#define CERTIFI_ISSU_BACK   0x5A

//安全芯片证书制发指令为0x63，响应为0x64;
//安全芯片复位指令0x65，响应为0x66;

#define RZ_READER      0x61
#define RZ_READER_BACK 0x62

#define MADE_AQZS        0x63
#define MADE_AQZS_BACK   0x64

#define RESET_SAFEIC      0x65
#define RESET_SAFEIC_BACK 0x66

#define GET_READERID      0x67
#define GET_READERID_BACK 0x68

#define SET_READERID      0x69
#define SET_READERID_BACK 0x6A

#define	SET_GATE     0x6B	//
#define	SET_GATE_BACK     0x6C	//

#define SET_FHM      0x6D
#define SET_FHM_BACK 0x6E

#define SET_RFADD  0x6F
#define SET_RFADD_BACK 0x70

#define GET_RFADD  0x71
#define GET_RFADD_BACK 0x72

#define GET_GATE  0x73
#define GET_GATE_BACK 0x74

#define SET_SLEEP_TIME 0x75
#define SET_SLEEP_TIME_BACK 0x76

#define SET_GPS_MODE 0x77
#define SET_GPS_MODE_BACK 0x78

//LockStatus
#define	LOCKWAIT  0X10   //待机
#define	SEAL      0X20   //施封
#define	UNSEAL    0X30   //解封
#define	ALARM     0X40   //报警
#define	EXALARM   0X50   //解警

//AlarmType
#define CUTALARM    0X01
#define OPENALARM   0X02//
#define CKALARM     0X04//
#define YJKSALARM   0X08//

#define cALERT		     0X01//
#define cGPRS_TRLIST   0X02//
#define	cMONITOR_LIST  0X03//

#define bLOCK     0
#define bUnLOCK   1
//-----------------GB FLAG BEGIN-----------------//
//LOCK_err_FLAG:
#define bNOT_OK   0X01
#define bOUT 			0X02
#define bVddLOW   0X04
#define bAGAIN    0X08
#define bCKBJ			0X10
#define bOP_OT    0X20

//unLOCK_err_FLAG:
//#define bNOT_OK   0X01
#define bKEYERR 	0X02
//#define bVddLOW   0X04
//#define bAGAIN    0X08
#define bNOTSEAL	0X10
#define bALARM    0X20
#define bUN_OT    0X40

//CLR_ALARM_err_FLAG:
#define bCLR_OK    0X01
//#define bKEYERR    0X02
#define bNO_ALARM  0X03

#define VERSION1 "+LOCKRFGB03+4521\r\n\0"
#define VERSION2 "H3.0KhGuGRLV5.00-4521X5\r\n\0"
#define VERSION3 "SRD-TA04-31-FW1.00.003\r\n\0"

//TT_Alarm_Status报警位：4wei
#define aLockOPEN 	0x04//BIT 2=1，锁杆剪断
#define aVDDLOW   	0x08//BIT 3=1，电压低报警
#define aLockCUT  	0x20//BIT 5=1，拆壳报警
#define ayjks  			0x80//BIT 7=1，非法开锁

//extern ///////////////////////////////
extern const u8 WNMM[];

extern u32 LockTicker;
extern u8 TT_Alarm_Status;
extern u8 Readyrstcnt;
extern u8 NetModuleSTATUS;
extern u8 NeedSendAlarm;
extern u8 Tcpsendflag;
extern u8 tcpsendtick;
extern u8 qisacktick;

extern unsigned int StartTime;
extern u8 GB_LOCKIDbuf[9];
extern u8 LockStatus;
extern u8 MoToStatus;
extern u8 AlarmType;
extern u8 RFTxBuf[100];
extern u8 AlarmCnt;
extern u8 AlarmTimebuf[8];
extern u32 SeqCN;
extern u8 TotalRecNum1;
extern u16 RecordIndex;
extern u8 SecKeyBuf[11];
extern u8 SecKeyBuf6[6];
extern u16 RecReTrTimer;
extern u16 NewReTrTimer;
extern u16 SysTimer;
extern u8 Vdd;
extern u8 GBVdd;
extern u8 Needresetflag[3];
extern u16 Adc_Vdd;
extern u8 Send2CCBuf[100];
extern u8 CheckLowPower;
extern u8 CheckLowStep;
extern u8 ACC_Off_flag;
extern u16 Lowpowertick;
extern u8 TCPIP_been_OK;
extern u8 GPRSReConnectCnt;
extern u8 ModuleNeedCutRST;
extern u8 ModuleTempClosedCnt;
extern u8 HadCut_MPW_BOLV;
extern u8 GPS_ON_Flag;
extern u8 GPS_OFF_Flag;
extern u8 SMS_in_Num[20];
extern u8 Tick_250ms;
extern u8 GPRS_LP_flag;//20180201添加，各个模块休眠标志，0不休眠，1进入休眠
extern u8 GPS_LP_flag;
extern u8 KEY_LP_flag;
extern u8 RF_LP_flag;	//20180201添加，各部分休眠标志，0不休眠，1进入休眠
extern u8 vddlow_flag;

void Lock_EProm_Reset(u8 mode);
void LockTansGBLOCKID(void);
void Lock_Para_Init(void);
void Lock_Para_Get(void);
void LockTimHandler(void);
u8 GBRec_FIFO(u8 *recbuf,u8 newbc,u8 mode);
u8 GBRec_Center_FIFO(u8 *recbuf,u8 newbc,u8 mode);
u8 MotoDriver(u8 type);
u8 ReUnLock(void);
void ShowDateTime(u8* stime,u8* dtime);
void LockOPEN_OP(void);
void LockCHAIKE_OP(void);
void LockYJKS_OP(void);
void DLcmdCHK(u8 *rmbuf,u8 rmlen,u8 port);
void IOCheckHandler(void);
void  VinsideHandler(void);
u8 EEPROM_OP(u8 *datbuf,u32 address,u16 num,u8 mode);
void PsecondEventHandler(void);
void UserLoopTask(void);
void CheckForSleep(void);
#endif
