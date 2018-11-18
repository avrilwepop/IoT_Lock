#ifndef  __BSprotocol_h_
#define  __BSprotocol_h_

#include 	"config.h"

#define DNS_ADDR0 "AT+QIDNSGIP=\"server.siruide.net\"\r\0"
#define TCP_SERVER0 "AT+QIOPEN=0,\"TCP\",\"058.214.242.018\",8888\r\0"		//  101.132.168.166\",5088
#define UDP_SERVER0 "AT+QIOPEN=1,\"UDP\",\"058.214.242.018\",8888\r\0"

#define TCPCHARLEN  43
#define TCP_SERVER3 "AT+QIOPEN=0,\"TCP\",\"203.080.144.166\",5088\r\0"
#define UDP_SERVER3 "AT+QIOPEN=1,\"UDP\",\"203.080.144.166\",5089\r\0"

#define APN_PARA0 "AT+QICSGP=1,\"CMNET\"\r\0"
#define APN_PARA3 "AT+QICSGP=1,\"CMNET\"\r\0"

#define TCP_PORT0 "5088\r\0"
#define UDP_PORT0 "5089\r\0"

#define GPRS_DATA     2

#define SELF_OPR  		(0)
#define KEY_OPR  			(1)
#define OFFLINE_OPR  	(2)
#define SERVER_OPR  	(3)

//=======================================cmd==================================================
#define SUBPACKAGE_FLAG (0x2000)
#define MSGLENBIT (0x01FF)

#define SERVER_ANSWER  					(0x8001)
#define DEVICE_ANSWER  					(0x0001)
#define DEVICE_HEARTTCK  				(0x0002)
#define DEVICE_REGISTER  				(0x0100)
#define DEVICE_REGISTER_ANSWER  	(0x8100)
#define DEVICE_CANCEL  					(0x0003)
#define DEVICE_AUTHEN  					(0x0102)
#define SET_DEVICE_PARAM  				(0x8103)
#define GET_DEVICE_ALLPARAM  		(0x8104)
#define GET_DEVICE_SPEPARAM  		(0x8106)
#define GET_DEVICE_PARAM_ANSWER  (0x0104)
#define DEVICE_LOCATION  				(0x0200)
#define CHECK_LOCATION  					(0x8201)
#define CHECK_LOCATION_ANSWER  	(0x0201)
#define DEVICE_OFFLINELOCATION  	(0x0704)

#define DEVICE_UNLOCK  					(0x8F00)
#define DEVICE_UNLOCK_BACK  			(0x0F00)
#define DEVICE_LOCK  						(0x8F01)
#define DEVICE_LOCK_BACK  				(0x0F01)
#define DEVICE_CLRALARM  				(0x8F02)
#define DEVICE_CLRALARM_BACK  		(0x0F02)

#define DEVICE_RF_KEY_UNLOCK_BACK (0x0F03)
#define DEVICE_RF_LOCK_BACK  		(0x0F04)
#define DEVICE_KEY_LOCK_BACK  		(0x0F05)
#define DEVICE_RF_CLRALARM_BACK  (0x0F06)



typedef struct 
{
	u8 retrytick;
	u8 authresult;
	u8 indexcode[2];
	u8 datalen;
	u8 authprotodata[45];
}authorize_protocol;

typedef struct 
{
	u8 regiStatus;  //设备注册标志，Y, 已注册；  N  未注册
	u8 lince[15];   //授权码
	u8 snid[6];    // 手机号
	u32 GprsRetrycount;   // 重发次数   5次
	u32 GprsRetryTimeout;   // 重发时间    5000ms
  	u16 Province; 
	u16 City;
	u8 Manufacturer[5];
	u8 Type[20];
	u8 DeviceID[7];
	u8 Colour;
	u8 VIN[7];
	u32 HandTick;
}dev_parameters;


typedef struct 
{	
	u16 MessageId;
	u16 MessageProperty;
	u8 SN_ID[6];
	u8 SerialIndex[2];
}server_msg_head;


typedef struct 
{
	u8 AnswerDeviceSerialIndex[2];
	u16 AnswerDeviceMessageId;
	u8 Result;
}server_answer_data;

typedef struct 
{
	u8 AnswerServerSerialIndex[2];
	u16 AnswerServerMessageId;
	u8 Result;
}device_answer_data;

//===============================================================
extern u32 gpsUPTimeLEN;
extern u32 gpsUPTimeLENsleep;
extern u16 IndexCode;

extern u8  DNS_ADDR[47];
extern u8  TCP_SERVER[43]; 
extern u8  TCP_SERVER2[43]; 
extern u8  UDP_SERVER[43];
extern u8  UDP_SERVER2[43];
extern u8  TCP_PORT[6];
extern u8  UDP_PORT[6];
extern u8  APN_PARA[64];
extern u8  APN_PARA2[64];
extern authorize_protocol AuthorizeProtocol;
extern dev_parameters DevParameters;

void E2PROM_check(void);
void DeviceParamentInit(void);
void TCPIPTimerHandler(void);
void HandlePacket(void);
void RcvDataOP(u8* abuf,u8 len,u8 linknum);
void Muc1_Process(u8 *Buff,u8 Type);
u8 IPset(u8 cmd,u8* src,u8 Type);
u16 TransferSendData(u8* abuf,u16 len);
u16 TransferReceiveData(u8* abuf,u16 len);
void DataFenBao(u8 * abuf,u8 asclen,u8 link);
void NewPacket_KY(u8* src,u8 len,u16 cmd,u8 commport);
void DeviceRegisteOperate(void);

#endif


