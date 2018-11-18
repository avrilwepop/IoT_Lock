#ifndef  __gbprotocol_h_
#define  __gbprotocol_h_

#include 	"config.h"

#define GB_XY  0X7B
#define XYID  GB_XY
#define HEAD  0xFF

#define bJAM  0X10
#define bP2P  0X02
#define JIAM  0XAA
#define JIEM  0XDD
#define bUP   0X80

//=====================EEPROM ����==========================//

#define LOCKSN_ADDR							896								//SN�� 6�ֽ�
#define GB_LOCKID_ADDR 					LOCKSN_ADDR+6 		//8�ֽ�

//IP1��ַ
#define TCP_ADDR1               	0 								//48B
#define UDP_ADDR1               	TCP_ADDR1+48 			//48B
#define TCP_PORT_ADDR1    				UDP_ADDR1+48  		//6B
#define UDP_PORT_ADDR1    				TCP_PORT_ADDR1+6  //6B
#define APN_ADDR1               	128 							//65B
//IP2��ַ
#define TCP_ADDR2               	256 							//48B
#define UDP_ADDR2               	TCP_ADDR2+48 			//48B
#define TCP_PORT_ADDR2    				UDP_ADDR2+48			//6B
#define UDP_PORT_ADDR2    				UDP_PORT_ADDR2+6	//6B
#define APN_ADDR2               	384								//65B
//����
#define DNS_ADDR1               	512 							//48B
#define TCP_PORT_ADDR3    				DNS_ADDR1+48			//6B
#define UDP_PORT_ADDR3    				TCP_PORT_ADDR3+6	//6B
#define APN_ADDR3             	 	640								//65B

//�������洢��ַ
#define DEVPAR_BASE_ADD    			768   //  �����洢��ַ��ַ
#define DEVPAR_BYTES_NUM    			78      //����ռ�ֽ���
#define DEV_REGISTATUS_ADD   			(0x0000+DEVPAR_BASE_ADD) //ע��״̬			1�ֽ�
#define DEV_LINCE_ADD 							(0x0001+DEVPAR_BASE_ADD) //��Ȩ��				15�ֽ�
#define DEV_SIMID_ADD   						(0x0010+DEVPAR_BASE_ADD) //�ֻ�����			6�ֽ�
#define DEV_TCPRETRYCOUNT_ADDR 	 	(0x0016+DEVPAR_BASE_ADD) //TCP��������	4�ֽ�
#define DEV_TCPRETRYTIMEOUT_ADDR  	(0x001A+DEVPAR_BASE_ADD) //TCP����ʱ��	4�ֽ�
#define DEV_PROVINCE_ADDR 	 				(0x001E+DEVPAR_BASE_ADD) //ʡ��ID				2�ֽ�
#define DEV_CITY_ADDR  						(0x0020+DEVPAR_BASE_ADD) //����ID				2�ֽ�
#define DEV_MANUFAC_ADDR 	 				(0x0022+DEVPAR_BASE_ADD) //������ID			5�ֽ�
#define DEV_TYPE_ADDR  						(0x0027+DEVPAR_BASE_ADD) //�ն��ͺ�			20�ֽ�
#define DEV_DEVICEID_ADDR 	 				(0x003B+DEVPAR_BASE_ADD) //�ն�ID				7�ֽ�
#define DEV_COLOUR_ADDR  					(0x0042+DEVPAR_BASE_ADD) //������ɫ			1�ֽ�
#define DEV_VIN_ADDR  							(0x0043+DEVPAR_BASE_ADD) //VIN��				7�ֽ�
#define DEV_HANDTICK_TIMELEN_ADDR 	(0x004A+DEVPAR_BASE_ADD) //������� 		4�ֽ�

//���²�������˳��洢����
#define gpsUPTimeLEN_ADDR       	1024	//4
#define gpsUPTimeLENsleep_ADDR  	1028	//4
#define TT_Alarm_Status_ADDR    	1032 	//1
#define Needresetflag_ADDR 			1055	//3
#define TotalRecNum_ADDR  				1058	//2
#define LockStatus_ADDR   				1060	//1
#define MoToStatus_ADDR   				1061	//1
#define AlarmType_ADDR    				1062	//1
#define AlarmTime_ADDR    				1063	//8
#define SecKey_ADDR       				1071	//10
#define SecKey_ADDR2      				1081	//10
#define SecKey6_ADDR       			1091	//6
#define SecKey6_ADDR2       			1097	//6
#define SeqCN_ADDR     					1103	//4
#define RECORDINDEX_ADDR 				1107	//2
#define OFFLINEGPSINDEX_ADDR 		1109	//2

//32����������¼//
#define MAXBC_NUM								30//����¼����
#define LockEVENT1_ADDR 					2048  //8b*32 = 256
#define LockNEWEVENT_ADDR 				2048 

//3000��GPSä��洢 �׵�ַ
#define OFFLINEGPSBASE  12288  //3000*16 = 48000  


//���籣�ֵ�����
#define TAGRECORDBUFF 4096   //MAXLockRECVOL*HVOL 20*120 = 0x960    0x1800 ~ 02160   20
 
//=========================GB RF cmd======================================/
#define	GB_LOCK_INFO_UP		  0X40 //��ȫ����������Ƶ���ѵ�������Ӧ

#define	GB_LOCK						  0X41 //ʩ��
#define	GB_LOCK_BACK				0X42 //ʩ����Ӧ
#define	GB_UNLOCK					  0X43 //��� 
#define	GB_UNLOCK_BACK			0X44 //�����Ӧ
#define	GB_CHECK						0X45 //���
#define	GB_CHECK_BACK				0X46 //�����Ӧ
#define	GB_CLR_ALARM				0X47 //������� 
#define	GB_CLR_ALARM_BACK		0X48 //���������Ӧ

#define	GB_WR_EXDATA	      0x49	//д��չ����
#define	GB_WR_EXDATA_BACK   0x4A	//д��չ������Ӧ
#define	GB_RD_EXDATA 				0x4B	//����չ����
#define	GB_RD_EXDATA_BACK		0x4C	//����չ������Ӧ
#define	GB_RD_EVENTDATA     0x4D	//���¼�����
#define	GB_RD_EVENTDATA_BACK 0x4E	//���¼�������Ӧ
#define	GB_WR_CID          	0x4F	//д��/����
#define	GB_WR_CID_BACK			0x50	//д��/������Ӧ
#define	GB_RD_CID 					0x51	//����/����
#define	GB_RD_CID_BACK	    0x52	//����/������Ӧ

#define	GB_DL_READER_PKEY   0x53	//�´��Ķ�����Կ
#define	GB_DL_READER_PKEY_BACK   0x54	//�´��Ķ�����Կ��Ӧ
#define	GB_READ_LOCK_PKEY   0x55	//��ȡ����Կ
#define	GB_READ_LOCK_PKEY_BACK   0x56	//��ȡ����Կ��Ӧ
//0x57-7F	//����
//0x80-BF	//�����̵���ָ��
//0xC0-FF	//����

#define 	NEWRF_CHECK					0X81  
#define	NEWRF_CHECK_BACK			0X82
#define	NEWRF_RD_EVENTDATA 			0x83	 
#define	NEWRF_RD_EVENTDATA_BACK		0x84
#define	NEWRF_UNLOCK				0X85  
#define	NEWRF_UNLOCK_BACK			0X86  
#define	NEWRF_LOCK					0X87  
#define	NEWRF_LOCK_BACK				0X88 
#define	NEWRF_CLR_ALARM				0X89  
#define	NEWRF_CLR_ALARM_BACK		0X8A  
	 
#define	PRELOCK     			 0X80	//Ԥʩ��//
#define	PRELOCK_BACK       0X81	 

#define	GB_GPSGSM_OFF          0X82	//
#define	GB_GPSGSM_OFF_BACK     0X83	//
#define	GB_GPSGSM_ON           0X84	//
#define	GB_GPSGSM_ON_BACK      0X85	//
#define	GB_GPSOFF_GSMON        0X86	//
#define	GB_GPSOFF_GSMON_BACK   0X87	//

#define	GB_RD_CHECKFLAG          0X88	//
#define	GB_RD_CHECKFLAG_BACK     0X89	//

#define	GB_WR_CHECKFLAG          0X8A	//
#define	GB_WR_CHECKFLAG_BACK     0X8B	//

#define	GB_NeedCheckKEY_YES          0X8C	//
#define	GB_NeedCheckKEY_YES_BACK     0X8D	//
#define	GB_NeedCheckKEY_NO           0X8E	//
#define	GB_NeedCheckKEY_NO_BACK      0X8F	//

#define	GB_NeedLoWPower           0X90	//
#define	GB_NeedLoWPower_BACK      0X91	//

#define	GB_NeedGprsOperate          0X92	//
#define	GB_NeedGprsOperate_BACK      0X93	//
#define	GB_SetSIMNumber          0X94	//
#define	GB_SetSIMNumber_BACK      0X95	//

#define	GB_YJCLR_ALARM				  0X96 //������� 
#define	GB_YJCLR_ALARM_BACK		0X97 //���������Ӧ

#define GB_RF_TEST 0xC0

//OP_EVENT//
#define GB_LOCK_EVENT 0X01
#define GB_UNLOCK_EVENT 0X02
#define GB_CLRALARM_EVENT 0X03
#define GB_OPENALARM_EVENT 0X04
#define GB_CUTALARM_EVENT 0X05
#define GB_LOWVDD_EVENT 0X06
#define GB_KEYERR_EVENT 0X07
#define GB_CKALARM_EVENT 0X08
#define GB_YJKSALARM_EVENT 0X09

#define NEW_UNLOCK_GPRS_EVENT 0X00
#define NEW_UNLOCK_RFID_EVENT 0X01
#define NEW_UNLOCK_KEY_EVENT  0X02
#define NEW_CLRALARM_EVENT 0X03
#define NEW_CHECK_EVENT 0X04
#define NEW_MESERR_EVENT 0X05
#define NEW_LOWVDD_EVENT 0X06
#define NEW_OPENALARM_EVENT 0X07
#define NEW_CKALARM_EVENT 0X08

#define NEW_KEYERR_EVENT 0X09
#define NEW_LOCK_GPRS_EVENT 0X0A
#define NEW_LOCK_RFID_EVENT 0X0B
#define NEW_LOCK_KEY_EVENT  0X0C

#define RF_PTC_VER 0x02

#define YJKS_ALARM_UP  0X02		  //TT
#define CK_ALARM_UP	   0X03
#define CUT_ALARM_UP   0X04
#define OPEN_ALARM_UP  0X05
#define PHST_ALARM_UP  0x06
#define LOWVDD_UP      0x0B

#endif

