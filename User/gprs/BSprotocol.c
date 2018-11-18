#include "common.h"
#include "TTLM.H"
#include "gbprotocol.h"
#include "gprs.h"
#include "BSprotocol.h"
#include "crcencoder.h"
#include "rfid.h"
#include "gps.h"

u8 ipstr[15]= {'0','0','0','.','0','0','0','.','0','0','0','.','0','0','0'};

u32 gpsUPTimeLEN = 30;//定时参数eep...
u32 gpsUPTimeLENsleep = 3600;//定时参数eep...
u16 IndexCode = 0;
u8  DNS_ADDR[47];
u8  TCP_SERVER[43]; 
u8  TCP_SERVER2[43]; 
u8  UDP_SERVER[43];
u8  UDP_SERVER2[43];
u8  TCP_PORT[6];
u8  UDP_PORT[6];
u8  APN_PARA[64];
u8  APN_PARA2[64];
u8 ACK_Code[3];
dev_parameters DevParameters;
authorize_protocol AuthorizeProtocol;

/*******************************************************************************
* Function Name  : E2PROM_check
* Description    : 初始化值
* Input          :  
* Return         :  
*******************************************************************************/
void E2PROM_check(void)
{
	u16 i,j,k;

	EEPROM_OP((u8*)&TT_Alarm_Status,TT_Alarm_Status_ADDR,1,MODE_R);

	EEPROM_OP((u8*)&gpsUPTimeLEN,gpsUPTimeLEN_ADDR,4,MODE_R);
	if(gpsUPTimeLEN==0xFFFFFFFF)
	{
		gpsUPTimeLEN=30;   //30s
		EEPROM_OP((u8*)&gpsUPTimeLEN,gpsUPTimeLEN_ADDR,4,MODE_W);
	}
	EEPROM_OP((u8*)&gpsUPTimeLENsleep,gpsUPTimeLENsleep_ADDR,4,MODE_R);
	if(gpsUPTimeLENsleep==0xFFFFFFFF)
	{
		gpsUPTimeLENsleep=3600;   //3600s
		EEPROM_OP((u8*)&gpsUPTimeLENsleep,gpsUPTimeLENsleep_ADDR,4,MODE_W);
	}
	EEPROM_OP((u8*)DNS_ADDR,DNS_ADDR1,48,MODE_R);
	//UARTWrite((u8 *)DNS_ADDR,str_len(DNS_ADDR),DEBUG_COM);//debuggg

	i = 0;
	if(memCpare((u8*)DNS_ADDR,(u8*)"AT+QIDNSGIP=\"\0",13))
	{
		i = 100;
	}
	EEPROM_OP((u8*)TCP_PORT,TCP_PORT_ADDR1,6,MODE_R);
	//UARTWrite((u8 *)TCP_PORT,6,DEBUG_COM);//debuggg
	EEPROM_OP((u8*)UDP_PORT,UDP_PORT_ADDR1,6,MODE_R);
	//UARTWrite((u8 *)UDP_PORT,6,DEBUG_COM);//debuggg

	EEPROM_OP((u8*)TCP_SERVER,TCP_ADDR1,42,MODE_R);
	//UARTWrite((u8 *)TCP_SERVER,42,DEBUG_COM);//debuggg
	//UARTWrite((u8 *)"\r\n",2,DEBUG_COM);//debuggg

	j=0;
	if(memCpare((u8*)TCP_SERVER,(u8*)TCP_SERVER0,17))
	{
		EEPROM_OP((u8*)UDP_SERVER,UDP_ADDR1,TCPCHARLEN,MODE_R);
		//UARTWrite((u8 *)UDP_SERVER,TCPCHARLEN,DEBUG_COM);//debuggg
		//UARTWrite((u8 *)"\r\n",2,DEBUG_COM);//debuggg
		if(memCpare((u8*)UDP_SERVER,(u8*)UDP_SERVER0,17))
		{
			EEPROM_OP((u8*)APN_PARA,APN_ADDR1,64,MODE_R);
			//UARTWrite((u8 *)APN_PARA,str_len(APN_PARA),DEBUG_COM);//debuggg

			if(memCpare((u8*)APN_PARA,(u8*)"AT+QICSGP=1,\"\0",13))
			{
				j=100;//ok//
			}
		}
	}
	EEPROM_OP((u8*)TCP_SERVER2,TCP_ADDR2,TCPCHARLEN,MODE_R);
	//UARTWrite((u8 *)TCP_SERVER2,TCPCHARLEN,DEBUG_COM);//debuggg
	//UARTWrite((u8 *)"\r\n",2,DEBUG_COM);//debuggg
	k = 0;
	if(memCpare((u8*)TCP_SERVER2,(u8*)"AT+QIOPEN=0,\"TCP\"\0",17))
	{
		EEPROM_OP((u8*)UDP_SERVER2,UDP_ADDR2,TCPCHARLEN,MODE_R);
		//UARTWrite((u8 *)UDP_SERVER2,TCPCHARLEN,DEBUG_COM);//debuggg
		//UARTWrite((u8 *)"\r\n",2,DEBUG_COM);//debuggg
		if(memCpare((u8*)UDP_SERVER2,(u8*)"AT+QIOPEN=1,\"UDP\"\0",17))
		{
			EEPROM_OP((u8*)APN_PARA2,APN_ADDR2,64,MODE_R);
			//UARTWrite((u8 *)APN_PARA2,str_len(APN_PARA2),DEBUG_COM);//debuggg
			//UARTWrite((u8 *)"\r\n",2,DEBUG_COM);//debuggg
			if(memCpare((u8*)APN_PARA2,(u8*)"AT+QICSGP=1,\"\0",13))
			{
				k=100;//ok//
			}
		}
	}
	if((i!= 100)||(j!=100)||(k!=100))
	{
		//UARTWrite((u8 *)"恢复IP0\r\n",9,DEBUG_COM);
		IPset(0x30,ACK_Code,0); 
	}
	DeviceParamentInit();
}
/*******************************************************************************
* Function Name  : DeviceParamentInit
* Description    : GPRS协议参数的初始化
* Input          :  
* Return         :  
*******************************************************************************/
void DeviceParamentInit(void)
{
	u8 tmp[80];
	u32 di;
	
	memREset((u8*)&AuthorizeProtocol,0,sizeof(AuthorizeProtocol));

	EEPROM_OP(tmp,DEVPAR_BASE_ADD,DEVPAR_BYTES_NUM,MODE_R);
	if((tmp[0] != 'Y')&&(tmp[0] != 'N'))
	{
		tmp[0] = 'N';
		EEPROM_OP((u8*)&tmp[0],DEV_REGISTATUS_ADD,1,MODE_W);
	}
	DevParameters.regiStatus = tmp[0];

	if((tmp[1] != 11)&&(tmp[1] != 0))
	{
		memREset((u8*)&(DevParameters.lince[0]),0,15);
		EEPROM_OP((u8*)&DevParameters.lince[0],DEV_LINCE_ADD,15,MODE_W);
	}
	else if((tmp[1] == 11)&&(tmp[0] == 'Y'))
		memcopy((u8*)&DevParameters.lince[0],(u8*)&tmp[1],(tmp[1]+1));
	else	
	{
		//memREset((u8*)&(DevParameters.lince[0]),0,15);
		DevParameters.lince[0]=3;
		DevParameters.lince[1]=0x31;
		DevParameters.lince[2]=0x32;
		DevParameters.lince[3]=0x33;
	}

	if(tmp[1+15] != 0x01)
	{
		tmp[1+15] = 0x01;	tmp[1+15+1] = 0x90;
		tmp[1+15+2] = 0x00;	tmp[1+15+3] = 0x00;
		tmp[1+15+4] = 0x00;	tmp[1+15+5] = 0x01;
		EEPROM_OP((u8*)&tmp[1+15],DEV_SIMID_ADD,6,MODE_W);
	}
	memcopy((u8*)&DevParameters.snid[0],(u8*)&tmp[1+15],6);

	SendData2TestCOM((u8*)&DevParameters.snid,6,0); //for debug

	di = tmp[25]+((u32)tmp[24]<<8) +((u32)tmp[23]<<16) + ((u32)tmp[22]<<24);
	if(di >2)
	{
		di = 2;
		tmp[22] = 0;tmp[23] = 0;tmp[24] = 0;tmp[25] = 2;
		EEPROM_OP((u8*)&tmp[22],DEV_TCPRETRYCOUNT_ADDR,4,MODE_W);
	}
	DevParameters.GprsRetrycount = di;
	di = tmp[29] + ((u32)tmp[28]<<8)+((u32)tmp[27]<<16) + ((u32)tmp[26]<<24);
	if((di<=5)||(di>=60))
	{
		di = 0x05;
		tmp[26] = 0x00;
		tmp[27] = 0x00;
		tmp[28] = 0x00;
		tmp[29] = 0x05;
		EEPROM_OP((u8*)&tmp[26],DEV_TCPRETRYTIMEOUT_ADDR,4,MODE_W);
	}
	DevParameters.GprsRetryTimeout = di;

	if((tmp[31]< 0x11)||(tmp[31]> 0x82))
	{
		tmp[30] = 0x00;
		tmp[31]= 0x32;
		EEPROM_OP((u8*)&tmp[30],DEV_PROVINCE_ADDR,2,MODE_W);
	}
	DevParameters.Province= tmp[31]+((u16)tmp[30]<<8);
	
	if((tmp[32]< 0x01)||(tmp[32]> 0x09))
	{
		tmp[32] = 0x01;
		tmp[33] = 0x02;
		EEPROM_OP((u8*)&tmp[32],DEV_CITY_ADDR,2,MODE_W);
	}
	DevParameters.City= tmp[33]+((u16)tmp[32]<<8);

	di = (u32)tmp[77]+ ((u32)tmp[76]<<8)+((u32)tmp[75]<<16)+((u32)tmp[74]<<24);
	if((di == 0)||(di ==0xffffffff))
	{
		di =  0xB4;
		tmp[74] = 0;tmp[75] = 0;tmp[76] = 0x00;tmp[77] = 0xB4;
		EEPROM_OP((u8*)&tmp[70],DEV_HANDTICK_TIMELEN_ADDR,4,MODE_W);
	}
	DevParameters.HandTick = di;
}

/*******************************************************************************
* Function Name  : TCPIPTimerHandler
* Description    : 定时的心跳包发送
* Input          :  TCPIP_been_OK  TCP连接正常的标志
* Return         :  
*******************************************************************************/
void TCPIPTimerHandler(void)
{
	if((TCPIP_been_OK&0X01)==0X01)
	{
		if(++HandTick>=((DevParameters.HandTick)*4))
		{
			HandTick=0;
			HandlePacket();
		}
	}
}
/*******************************************************************************
* Function Name  : HandlePacket
* Description    : 发送心跳包
* Input          :  
* Return         :  
*******************************************************************************/
void HandlePacket(void)
{
	NewPacket_KY(0,0,DEVICE_HEARTTCK,TCPLINK);		
}

/*******************************************************************************
* Function Name  : DeviceRegisteOperate
* Description    : 设备注册
* Input          :  
* Return         :  
*******************************************************************************/
void DeviceRegisteOperate(void)
{
	u8 tmp[45],index;

	index = 0;
	tmp[index] = (u8)((DevParameters.Province)&(0xff00)>>8);
	index++;
	tmp[index] = (u8)((DevParameters.Province)&0xff);
	index++;
	tmp[index] = (u8)((DevParameters.City)&(0xff00)>>8);
	index++;
	tmp[index] = (u8)((DevParameters.City)&0xff);
	index++;
	memcopy(tmp+index,DevParameters.Manufacturer,5);
	index+=5;
	memcopy(tmp+index,DevParameters.Type,20);
	index+=20;
	tmp[index] = 0;index++;
	memcopy(tmp+index,DevParameters.snid,6);
	index+=6;
	tmp[index] = DevParameters.Colour;
	index++;
	memcopy(tmp+index,DevParameters.VIN,7);
	index+=7;
	NewPacket_KY(tmp,index,DEVICE_REGISTER,TCPLINK);
}
/*******************************************************************************
* Function Name  : NewPacket_KY
* Description    : GPRS模块往后台发数据
* Input          :  
* Return         :  
*******************************************************************************/
void NewPacket_KY(u8* src,u8 len,u16 cmd,u8 commport)
{ 
	u8 hexbuf[512];
	u8 crc,varlen;
	u8 i,index,var,hexpt;
	
	index = 0;
	hexbuf[index] = 0x7E;
	index++;

//	if(cmd == DEVICE_ANSWER)  //0x0001 	
//	{
//		hexbuf[index] = ((DEVICE_ANSWER&0xff00)>>8);index++;hexbuf[index] = (DEVICE_ANSWER&0xff);index++;	   //消息ID
//	}
//	else if(cmd == DEVICE_HEARTTCK)  //0x0002 	
//	{
//		hexbuf[index] = ((DEVICE_HEARTTCK&0xff00)>>8);index++;hexbuf[index] = (DEVICE_HEARTTCK&0xff);index++;	   
//	}
//	else if(cmd == DEVICE_CANCEL)  //0x0003 	
//	{
//		hexbuf[index] = ((DEVICE_CANCEL&0xff00)>>8);index++;hexbuf[index] = (DEVICE_CANCEL&0xff);index++;	  
//	}
//	else if(cmd == DEVICE_REGISTER)  //0x0100 	
//	{
//		hexbuf[index] = ((DEVICE_REGISTER&0xff00)>>8);index++;hexbuf[index] = (DEVICE_REGISTER&0xff);index++;	   
//	}
//	else if(cmd == DEVICE_AUTHEN)  //0x0102 	
//	{
//		hexbuf[index] = ((DEVICE_AUTHEN&0xff00)>>8);index++;hexbuf[index] = (DEVICE_AUTHEN&0xff);index++;	  
//	}
//	else if(cmd == GET_DEVICE_PARAM_ANSWER)  //0x0104 	
//	{
//		hexbuf[index] = ((GET_DEVICE_PARAM_ANSWER&0xff00)>>8);index++;hexbuf[index] = (GET_DEVICE_PARAM_ANSWER&0xff);index++;	 
//	}
//	else if(cmd == DEVICE_LOCATION)  //0x0200 	
//	{
//		hexbuf[index] = ((DEVICE_LOCATION&0xff00)>>8);index++;hexbuf[index] = (DEVICE_LOCATION&0xff);index++;	  
//	}	
//	else if(cmd == CHECK_LOCATION_ANSWER)  //0x0201 	
//	{
//		hexbuf[index] = ((CHECK_LOCATION_ANSWER&0xff00)>>8);index++;hexbuf[index] = (CHECK_LOCATION_ANSWER&0xff);index++;	  
//	}
//	else if(cmd == DEVICE_UNLOCK_BACK)  //0x0F00 	
//	{
//		hexbuf[index] = ((DEVICE_UNLOCK_BACK&0xff00)>>8);index++;hexbuf[index] = (DEVICE_UNLOCK_BACK&0xff);index++;	  
//	}
//	else if(cmd == DEVICE_LOCK_BACK)  //0x0F01 	
//	{
//		hexbuf[index] = ((DEVICE_LOCK_BACK&0xff00)>>8);index++;hexbuf[index] = (DEVICE_LOCK_BACK&0xff);index++;	  
//	}
//	else if(cmd == DEVICE_CLRALARM_BACK)  //0x0F02 	
//	{
//		hexbuf[index] = ((DEVICE_CLRALARM_BACK&0xff00)>>8);index++;hexbuf[index] = (DEVICE_CLRALARM_BACK&0xff);index++;	  
//	}
//	else if(cmd == DEVICE_OFFLINELOCATION)  //0x0704 	
//	{
//		hexbuf[index] = ((DEVICE_OFFLINELOCATION&0xff00)>>8);index++;hexbuf[index] = (DEVICE_OFFLINELOCATION&0xff);index++;	  
//	}
//	else
//	{
//		return;
//	}
	
	hexbuf[index] = ((cmd&0xff00)>>8);index++;
	hexbuf[index] = (cmd&0xff);index++;   
	hexbuf[index] = 0x00;index++;
	hexbuf[index] = len;index++;	  //消息属性，只有长度信息
	for(i =0; i<6;i++)
	{
		hexbuf[index] = DevParameters.snid[i]; 		// 
		index++;
	}
	IndexCode++;
	hexbuf[index] = (IndexCode>>8)&0x00ff;index++;
	hexbuf[index] = (IndexCode&0x00ff);index++;		//流水号
	if(len>0)
	{
		memcopy(hexbuf+index,src,len);     //  消息体内容
		index +=len;
	}
	crc = Do_XOR(hexbuf+1,index - 1);
	hexbuf[index] = crc;index++;
	hexbuf[index] = 0x7E;index++;
	varlen = index;

	if(cmd == DEVICE_ANSWER)  //0x0001 	
	{
	}
	else if(cmd == DEVICE_HEARTTCK)  //0x0002 	
	{
	}
	else if(cmd == DEVICE_CANCEL)  //0x0003 	
	{
	}
	else if(cmd == DEVICE_REGISTER)  //0x0100 	
	{
	}
	else if(cmd == DEVICE_AUTHEN)  //0x0102 	
	{
		AuthorizeProtocol.indexcode[0] = (IndexCode>>8)&0x00ff;
		AuthorizeProtocol.indexcode[1] = (IndexCode&0x00ff);
		AuthorizeProtocol.retrytick = (u8)(DevParameters.GprsRetrycount);
		AuthorizeProtocol.authresult = 'N';
		AuthorizeProtocol.datalen = varlen;
		memcopy((u8*)&(AuthorizeProtocol.authprotodata[0]),hexbuf,AuthorizeProtocol.datalen);
		NewReTrTimer=WORDMS; 
		UARTWrite((u8*)"ST!",3,DEBUG_COM);
	}
	else if(cmd == GET_DEVICE_PARAM_ANSWER)  //0x0104 	
	{
	}
	else if(cmd == DEVICE_LOCATION)  //0x0200 	
	{
	}	
	else if(cmd == CHECK_LOCATION_ANSWER)  //0x0201 	
	{
	}		
	else if((cmd == DEVICE_KEY_LOCK_BACK)||(cmd == DEVICE_RF_LOCK_BACK)||(cmd == DEVICE_RF_KEY_UNLOCK_BACK)||(cmd == DEVICE_RF_CLRALARM_BACK))
	{
		for(var=0;var<MAXLockRECVOL;var++)
		{
			EEPROM_OP((u8 *)&hexpt,((u32)(TAGRECORDBUFF)+(u32)var*HVOL), 1, MODE_R);

		    if((hexpt>0)&&(hexpt<=RETRAN_TIMES)) 
			{
				continue;
			}
		    hexpt = 1;
			EEPROM_OP((u8 *)&hexpt,((u32)(TAGRECORDBUFF)+(u32)var*HVOL), 1, MODE_W);  //index
			EEPROM_OP((u8 *)&varlen,((u32)(TAGRECORDBUFF)+(u32)var*HVOL+1), 1, MODE_W);  //length
		    EEPROM_OP(hexbuf,((u32)(TAGRECORDBUFF)+(u32)var*HVOL+2), varlen, MODE_W);  //data

			TrIndex=var;
			RecReTrTimer=WORDMS; 
			UARTWrite((u8*)"ST!",3,DEBUG_COM);
			break;
		}
	}
	
	if((cmd != DEVICE_REGISTER)&&(cmd != DEVICE_AUTHEN)&&(cmd != DEVICE_CANCEL)&&(AuthorizeProtocol.authresult != 'Y'))    //未注册，除注册指令外其他指令不处理
		return;

	SendData2TestCOM(hexbuf,varlen,0);
	if(commport == UDPLINK)
	{
		if((TCPIP_been_OK&0X02)==0X02)
		{
			NewIPSendOP(hexbuf,varlen,commport);
		}
	}
	else if(commport == TCPLINK)
	{
		if((TCPIP_been_OK&0X01)==0X01)
		{
			NewIPSendOP(hexbuf,varlen,commport);
		}		  
	}
}
/*******************************************************************************
* Function Name  : RcvDataOP
* Description    : GPRS数据接收，解析函数  部标
* Input          :  
* Return         :  
*******************************************************************************/
void RcvDataOP(u8* abuf,u8 len,u8 linknum)
{
	u8 cmdbuf[250],hexpt,i,result[256],ipbuff[17],strbuff[70],apnbuff[24],username[10],password[8],portbuff[6];
	u8 index,resultindex;
	u32 parID,port;
	u8  parlen,comma[6],tmp[8];
	u8  parnum,j,k,m,x,y,codeindex;
	u8* p;
	u8 msglen;
	server_answer_data ServerAnswerData;
	device_answer_data DeviceAnswerdata;
	server_msg_head ServerMsgHead;

	if((len < 5)||(len > 253))
	{
		return;
	}
	memcopy(cmdbuf,abuf,len);
	hexpt=len;
	
	if((linknum==SIM1)||(linknum==SIM2))
	{
		ServerMsgHead.MessageId = cmdbuf[1] + ((u16)cmdbuf[0]<<8);   //消息ID  
		ServerMsgHead.MessageProperty = cmdbuf[3] + ((u16)cmdbuf[2]<<8);//消息体属性
		if(((ServerMsgHead.MessageProperty)&SUBPACKAGE_FLAG)!= 0)
		{
			UARTWrite((u8*)"SUBPAC\r\n",8,DEBUG_COM);
			return;
		}
		msglen = (u8)((ServerMsgHead.MessageProperty)&MSGLENBIT);
		if((msglen + 13)!= hexpt)
		{
			UARTWrite((u8*)"LENERR\r\n",8,DEBUG_COM);
			return;
		}
		for(i =0; i<6;i++)
			ServerMsgHead.SN_ID[i]= cmdbuf[4+i];		//锁SN号
		ServerMsgHead.SerialIndex[0] =cmdbuf[10];
		ServerMsgHead.SerialIndex[1] =cmdbuf[11];//后台下发的消息流水号

	//-----------------------------------
	switch(ServerMsgHead.MessageId)//消息ID
	{
		case SERVER_ANSWER:
			ServerAnswerData.AnswerDeviceSerialIndex[0] =  cmdbuf[12];
			ServerAnswerData.AnswerDeviceSerialIndex[1] =  cmdbuf[13];
			ServerAnswerData.AnswerDeviceMessageId =  cmdbuf[15]+((u16)cmdbuf[14]<<8);		
			ServerAnswerData.Result = cmdbuf[16];
			if((ServerAnswerData.AnswerDeviceMessageId == DEVICE_AUTHEN)   //授权，平台回复通用应答
				&&(ServerAnswerData.AnswerDeviceSerialIndex[0] == AuthorizeProtocol.indexcode[0])
				&&(ServerAnswerData.AnswerDeviceSerialIndex[1] == AuthorizeProtocol.indexcode[1]))
			{
				AuthorizeProtocol.retrytick = 0;
				if(ServerAnswerData.Result == 0)
				{
					AuthorizeProtocol.authresult = 'Y';  //chenggong
					UARTWrite((u8*)"授权成功\r\n",10,DEBUG_COM);
				}
				else
					AuthorizeProtocol.authresult = 0;  
			}
			else if(ServerAnswerData.AnswerDeviceMessageId == DEVICE_CANCEL)  //  注销，平台回复通用应答
			{
				if(ServerAnswerData.Result  == 0)
				{
					DevParameters.regiStatus = 'N';
					AuthorizeProtocol.authresult = 0;  
					memREset(DevParameters.lince,0,15);
					
					EEPROM_OP((u8*)&DevParameters.regiStatus,DEV_REGISTATUS_ADD,1,MODE_W);
					EEPROM_OP((u8*)&DevParameters.lince[0],DEV_LINCE_ADD,15,MODE_W);
				}
			}
			else if(ServerAnswerData.AnswerDeviceMessageId == DEVICE_HEARTTCK)
			{
				if(ServerAnswerData.Result  == 0)
				{
					UARTWrite((u8*)"心跳正常\r\n",10,DEBUG_COM);
				}
			}
			else if(ServerAnswerData.AnswerDeviceMessageId == DEVICE_LOCATION)
			{
				if(ServerAnswerData.Result  == 0)
				{
					UARTWrite((u8*)"定位正常\r\n",10,DEBUG_COM);
				}
			}
			else if((ServerAnswerData.AnswerDeviceMessageId == DEVICE_KEY_LOCK_BACK)
					||(ServerAnswerData.AnswerDeviceMessageId == DEVICE_RF_LOCK_BACK)
					||(ServerAnswerData.AnswerDeviceMessageId == DEVICE_RF_KEY_UNLOCK_BACK)
					||(ServerAnswerData.AnswerDeviceMessageId == DEVICE_RF_CLRALARM_BACK))
			{
				for(i = 0; i<MAXLockRECVOL;i++)
				{
					EEPROM_OP((u8 *)&ipbuff[0],((u32)(TAGRECORDBUFF)+(u32)i*HVOL), 1, MODE_R);

				    if((ipbuff[0]==0)&&(ipbuff[0]>RETRAN_TIMES)) //重发次数
					{
						continue;
					}
					EEPROM_OP((u8 *)&ipbuff[0],((u32)(TAGRECORDBUFF)+(u32)i*HVOL+13),2, MODE_R);  //第14 15 字节为流水号
					if((ipbuff[0]==ServerAnswerData.AnswerDeviceSerialIndex[0])&&(ipbuff[1]==ServerAnswerData.AnswerDeviceSerialIndex[1]))
					{
						ipbuff[0] = 0;
						EEPROM_OP((u8 *)&ipbuff[0],((u32)(TAGRECORDBUFF)+(u32)i*HVOL), 1, MODE_W);
						UARTWrite((u8*)"endretry\r\n",10,DEBUG_COM);
					}
				}
			}
			break;

		case DEVICE_REGISTER_ANSWER:
			switch(cmdbuf[14])
			{
				case 0x00:
					UARTWrite((u8*)"000\r\n",5,DEBUG_COM);   //debug
					for(i= 0;i<(msglen - 3);i++)
						DevParameters.lince[1+i] = cmdbuf[15+i];
					DevParameters.regiStatus = 'Y';
					DevParameters.lince[0] = msglen - 3;
					EEPROM_OP((u8*)&DevParameters.regiStatus,DEV_REGISTATUS_ADD,1,MODE_W);
					EEPROM_OP((u8*)&DevParameters.lince[0],DEV_LINCE_ADD,(DevParameters.lince[0]+1),MODE_W);
					
					break;
				case 0x01:
					UARTWrite((u8*)"111\r\n",5,DEBUG_COM);
					break;
				case 0x02:
					UARTWrite((u8*)"222\r\n",5,DEBUG_COM);
					break;
				case 0x03:
					UARTWrite((u8*)"333\r\n",5,DEBUG_COM);
					break;
				case 0x04:
					UARTWrite((u8*)"444\r\n",5,DEBUG_COM);
					break;
				default:
					break;
			}
			break;
		case SET_DEVICE_PARAM: 
			index = 0; 
			resultindex = 0;
			parnum = cmdbuf[12];	//参数总个数 
			DeviceAnswerdata.AnswerServerSerialIndex[0] = ServerMsgHead.SerialIndex[0];
			DeviceAnswerdata.AnswerServerSerialIndex[1] = ServerMsgHead.SerialIndex[1];
			DeviceAnswerdata.AnswerServerMessageId = ServerMsgHead.MessageId;
			DeviceAnswerdata.Result = 3;  //初始值暂定为 不支持
			for(i = 0;i < parnum;i++)
			{
				parID = cmdbuf[16+index]+ ((u32)cmdbuf[15+index]<<8)+((u32)cmdbuf[14+index]<<16)+((u32)cmdbuf[13+index]<<24);   //参数ID	
				parlen = cmdbuf[17+index];						//参数长度
				p = (u8*)&cmdbuf[18+index];
				index +=5;
				index += parlen;
				switch(parID)
				{
					case 0x0001:	  //心跳时间间隔
						if(parlen == 4)
						{
							DevParameters.HandTick = (*(p+3))+ ((u32)(*(p+2))<<8)+((u32)(*(p+1))<<16)+((u32)(*p)<<24);
							if(DevParameters.HandTick !=0)
							{
								EEPROM_OP((u8*)&DevParameters.HandTick,DEV_HANDTICK_TIMELEN_ADDR,4,MODE_W);
							}
							result[resultindex] = 0;
							resultindex++;
						}
						else
						{
							result[resultindex] = 1;
							resultindex++;
						}
						break;	
					case 0x0002:      //TCP重发时间间隔
						if(parlen == 4)
						{
							DevParameters.GprsRetryTimeout = (*(p+3))+ ((u32)(*(p+2))<<8)+((u32)(*(p+1))<<16)+((u32)(*p)<<24);
							if(DevParameters.GprsRetryTimeout !=0)
							{
								EEPROM_OP((u8*)&DevParameters.GprsRetryTimeout,DEV_TCPRETRYTIMEOUT_ADDR,4,MODE_W);
							}
							result[resultindex] = 0;
							resultindex++;
						}
						else
						{
							result[resultindex] = 1;
							resultindex++;
						}
						break;
					case 0x0003:      //TCP重发次数
						if(parlen == 4)
						{
							DevParameters.GprsRetrycount= (*(p+3))+ ((u32)(*(p+2))<<8)+((u32)(*(p+1))<<16)+((u32)(*p)<<24);
							if(DevParameters.GprsRetrycount !=0)
							{
								EEPROM_OP((u8*)&DevParameters.GprsRetrycount,DEV_TCPRETRYCOUNT_ADDR,4,MODE_W);
							}
							result[resultindex] = 0;
							resultindex++;
						}
						else
						{
							result[resultindex] = 1;
							resultindex++;
						}
						break;	
					case 0x0010:	  // APN 
						if((parlen >0 )&&(parlen <=24))
						{
							//AT+QICSGP=1,"CMNET","1234","567"
							for(k = 0 ;k<6;k++)
								comma[k] = 0;
							j = 0;
							apnbuff[0] = 0;
							username[0] = 0;
							password[0] = 0;
							EEPROM_OP((u8*)strbuff,APN_ADDR1,64,MODE_R);
							for(k = 0;i<64;k++)
							{
								if(strbuff[k] == 0x22)
								{
									comma[j] = k;
									j++;
								}
								if(strbuff[k] == '\r')
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
							memcopy(strbuff+13,p,parlen);
							strbuff[13+parlen] = '"';
							if(username[0] == 0)
							{
								strbuff[13+parlen+1] = ',';
								strbuff[13+parlen+2] = '"';
								strbuff[13+parlen+3] = '"';
							}
							else
							{
								strbuff[13+parlen+1] = ',';
								strbuff[13+parlen+2] = '"';
								memcopy(strbuff+13+parlen+3,username+1,username[0]);
								strbuff[13+parlen+3+username[0]] = '"';
							}
							if(password[0] ==0)
							{
								strbuff[13+parlen+3+username[0]+1] = ',';
								strbuff[13+parlen+3+username[0]+2] = '"';
								strbuff[13+parlen+3+username[0]+3] = '"';
								strbuff[13+parlen+3+username[0]+4] = '\r';
								strbuff[13+parlen+3+username[0]+5] = '\0';
							}
							else
							{
								strbuff[13+parlen+3+username[0]+1] = ',';
								strbuff[13+parlen+3+username[0]+2] = '"';
								memcopy(strbuff+13+parlen+3+username[0]+3,password+1,password[0]);
								strbuff[13+parlen+3+username[0]+3+password[0]] = '"';
								strbuff[13+parlen+3+username[0]+3+password[0]+1] = '\r';
								strbuff[13+parlen+3+username[0]+3+password[0]+2] = '\0';
							}
							EEPROM_OP((u8*)strbuff,APN_ADDR1,64,MODE_W);
							result[resultindex] = 0;
							resultindex++;
						}
						else
						{
							result[resultindex] = 1;
							resultindex++;
						}
						break;	
					case 0x0011:	  // APN--username
						if((parlen >0 )&&(parlen <=10))
						{
							//AT+QICSGP=1,"CMNET","1234","567"
							for(k = 0 ;k<6;k++)
								comma[k] = 0;
							j = 0;
							apnbuff[0] = 0;
							username[0] = 0;
							password[0] = 0;
							EEPROM_OP((u8*)strbuff,APN_ADDR1,64,MODE_R);
							for(k = 0;k<64;k++)
							{
								if(strbuff[k] == 0x22)
								{
									comma[j] = k;
									j++;
								}
								if(strbuff[k] == '\r')
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
							strbuff[13+apnbuff[0]+1] = ',';
							strbuff[13+apnbuff[0]+2] = '"';
							memcopy(strbuff+13+apnbuff[0]+3,p,parlen);
							strbuff[13+apnbuff[0]+3+parlen] = '"';
							if(password[0] ==0)
							{
								strbuff[13+apnbuff[0]+3+parlen+1] = ',';
								strbuff[13+apnbuff[0]+3+parlen+2] = '"';
								strbuff[13+apnbuff[0]+3+parlen+3] = '"';
								strbuff[13+apnbuff[0]+3+parlen+4] = '\r';
								strbuff[13+apnbuff[0]+3+parlen+5] = '\0';
							}
							else
							{
								strbuff[13+apnbuff[0]+3+parlen+1] = ',';
								strbuff[13+apnbuff[0]+3+parlen+2] = '"';
								memcopy(strbuff+13+apnbuff[0]+3+parlen+3,password+1,password[0]);
								strbuff[13+apnbuff[0]+3+parlen+3+password[0]] = '"';
								strbuff[13+apnbuff[0]+3+parlen+3+password[0]+1] = '\r';
								strbuff[13+apnbuff[0]+3+parlen+3+password[0]+2] = '\0';
							}
							EEPROM_OP((u8*)strbuff,APN_ADDR1,64,MODE_W);	
							result[resultindex] = 0;
							resultindex++;
						}
						else
						{
							result[resultindex] = 1;
							resultindex++;
						}
						break;	
					case 0x0012:	  // APN--password
						if((parlen >0 )&&(parlen <=8))
						{
							//AT+QICSGP=1,"CMNET","1234","567"
							for(k = 0 ;k<6;k++)
								comma[k] = 0;
							j = 0;
							apnbuff[0] = 0;
							username[0] = 0;
							password[0] = 0;
							EEPROM_OP((u8*)strbuff,APN_ADDR1,64,MODE_R);
							for(k = 0;k<64;k++)
							{
								if(strbuff[k] == 0x22)
								{
									comma[j] = k;
									j++;
								}
								if(strbuff[k] == '\r')
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
							if(username[0] == 0)
							{
								strbuff[13+apnbuff[0]+1] = ',';
								strbuff[13+apnbuff[0]+2] = '"';
								strbuff[13+apnbuff[0]+3] = '"';
							}
							else
							{
								strbuff[13+apnbuff[0]+1] = ',';
								strbuff[13+apnbuff[0]+2] = '"';
								memcopy(strbuff+13+apnbuff[0]+3,username+1,username[0]);
								strbuff[13+apnbuff[0]+3+username[0]] = '"';
							}
							strbuff[13+apnbuff[0]+3+username[0]+1] = ',';
							strbuff[13+apnbuff[0]+3+username[0]+2] = '"';
							memcopy(strbuff+13+apnbuff[0]+3+username[0]+3,p,parlen);
							strbuff[13+apnbuff[0]+3+username[0]+3+parlen] = '"';
							strbuff[13+apnbuff[0]+3+username[0]+3+parlen+1] = '\r';
							strbuff[13+apnbuff[0]+3+username[0]+3+parlen+2] = '\0';
							
							EEPROM_OP((u8*)strbuff,APN_ADDR1,64,MODE_W);	
							result[resultindex] = 0;
							resultindex++;
						}
						else
						{
							result[resultindex] = 1;
							resultindex++;
						}
						break;			
					case 0x0013:	 //IP1
						if((parlen >= 7)&&(parlen <= 15)) //x.x.x.x
						{
							memcopy(ipbuff,p,parlen);
							ipbuff[parlen] =  0x0d;
							ipbuff[parlen+1] =	0x0a;
							if(SearchIPchar(ipbuff,ipstr,17))
							{
								EEPROM_OP((u8*)TCP_SERVER,TCP_ADDR1,TCPCHARLEN,MODE_R);
								EEPROM_OP((u8*)UDP_SERVER,UDP_ADDR1,TCPCHARLEN,MODE_R);	
								memcopy(TCP_SERVER+19,ipstr,15);//ip
								TCP_SERVER[34]='"';
								memcopy(UDP_SERVER+19,ipstr,15);//ip
								UDP_SERVER[34]='"';
								UARTWrite((u8*)"+IP1set\r\n",9,DEBUG_COM);//debugggggg 							
								EEPROM_OP((u8*)TCP_SERVER,TCP_ADDR1,35,MODE_W);
								EEPROM_OP((u8*)UDP_SERVER,UDP_ADDR1,35,MODE_W);									
								result[resultindex] = 0;
								resultindex++;
							}
							else
							{
								result[resultindex] = 1;
								resultindex++;
							}
						}
						else
						{
							result[resultindex] = 1;
							resultindex++;
						}
						break;
					case 0x0014:	  // APN2 
						if((parlen >0 )&&(parlen <=24))
						{
							//AT+QICSGP=1,"CMNET","1234","567"
							for(k = 0 ;k<6;k++)
								comma[k] = 0;
							j = 0;
							apnbuff[0] = 0;
							username[0] = 0;
							password[0] = 0;
							EEPROM_OP((u8*)strbuff,APN_ADDR2,64,MODE_R);
							for(k = 0;i<64;k++)
							{
								if(strbuff[k] == 0x22)
								{
									comma[j] = k;
									j++;
								}
								if(strbuff[k] == '\r')
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
							memcopy(strbuff+13,p,parlen);
							strbuff[13+parlen] = '"';
							if(username[0] == 0)
							{
								strbuff[13+parlen+1] = ',';
								strbuff[13+parlen+2] = '"';
								strbuff[13+parlen+3] = '"';
							}
							else
							{
								strbuff[13+parlen+1] = ',';
								strbuff[13+parlen+2] = '"';
								memcopy(strbuff+13+parlen+3,username+1,username[0]);
								strbuff[13+parlen+3+username[0]] = '"';
							}
							if(password[0] ==0)
							{
								strbuff[13+parlen+3+username[0]+1] = ',';
								strbuff[13+parlen+3+username[0]+2] = '"';
								strbuff[13+parlen+3+username[0]+3] = '"';
								strbuff[13+parlen+3+username[0]+4] = '\r';
								strbuff[13+parlen+3+username[0]+5] = '\0';
							}
							else
							{
								strbuff[13+parlen+3+username[0]+1] = ',';
								strbuff[13+parlen+3+username[0]+2] = '"';
								memcopy(strbuff+13+parlen+3+username[0]+3,password+1,password[0]);
								strbuff[13+parlen+3+username[0]+3+password[0]] = '"';
								strbuff[13+parlen+3+username[0]+3+password[0]+1] = '\r';
								strbuff[13+parlen+3+username[0]+3+password[0]+2] = '\0';
							}
							EEPROM_OP((u8*)strbuff,APN_ADDR2,64,MODE_W);
							result[resultindex] = 0;
							resultindex++;
						}
						else
						{
							result[resultindex] = 1;
							resultindex++;
						}
						break;	
					case 0x0015:	  // APN2--username
						if((parlen >0 )&&(parlen <=10))
						{
							//AT+QICSGP=1,"CMNET","1234","567"
							for(k = 0 ;k<6;k++)
								comma[k] = 0;
							j = 0;
							apnbuff[0] = 0;
							username[0] = 0;
							password[0] = 0;
							EEPROM_OP((u8*)strbuff,APN_ADDR2,64,MODE_R);
							for(k = 0;k<64;k++)
							{
								if(strbuff[k] == 0x22)
								{
									comma[j] = k;
									j++;
								}
								if(strbuff[k] == '\r')
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
							strbuff[13+apnbuff[0]+1] = ',';
							strbuff[13+apnbuff[0]+2] = '"';
							memcopy(strbuff+13+apnbuff[0]+3,p,parlen);
							strbuff[13+apnbuff[0]+3+parlen] = '"';
							if(password[0] ==0)
							{
								strbuff[13+apnbuff[0]+3+parlen+1] = ',';
								strbuff[13+apnbuff[0]+3+parlen+2] = '"';
								strbuff[13+apnbuff[0]+3+parlen+3] = '"';
								strbuff[13+apnbuff[0]+3+parlen+4] = '\r';
								strbuff[13+apnbuff[0]+3+parlen+5] = '\0';
							}
							else
							{
								strbuff[13+apnbuff[0]+3+parlen+1] = ',';
								strbuff[13+apnbuff[0]+3+parlen+2] = '"';
								memcopy(strbuff+13+apnbuff[0]+3+parlen+3,password+1,password[0]);
								strbuff[13+apnbuff[0]+3+parlen+3+password[0]] = '"';
								strbuff[13+apnbuff[0]+3+parlen+3+password[0]+1] = '\r';
								strbuff[13+apnbuff[0]+3+parlen+3+password[0]+2] = '\0';
							}
							EEPROM_OP((u8*)strbuff,APN_ADDR2,64,MODE_W);	
							result[resultindex] = 0;
							resultindex++;
						}
						else
						{
							result[resultindex] = 1;
							resultindex++;
						}
						break;	
					case 0x0016:	  // APN--password
						if((parlen >0 )&&(parlen <=8))
						{
							//AT+QICSGP=1,"CMNET","1234","567"
							for(k = 0 ;k<6;k++)
								comma[k] = 0;
							j = 0;
							apnbuff[0] = 0;
							username[0] = 0;
							password[0] = 0;
							EEPROM_OP((u8*)strbuff,APN_ADDR2,64,MODE_R);
							for(k = 0;k<64;k++)
							{
								if(strbuff[k] == 0x22)
								{
									comma[j] = k;
									j++;
								}
								if(strbuff[k] == '\r')
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
							if(username[0] == 0)
							{
								strbuff[13+apnbuff[0]+1] = ',';
								strbuff[13+apnbuff[0]+2] = '"';
								strbuff[13+apnbuff[0]+3] = '"';
							}
							else
							{
								strbuff[13+apnbuff[0]+1] = ',';
								strbuff[13+apnbuff[0]+2] = '"';
								memcopy(strbuff+13+apnbuff[0]+3,username+1,username[0]);
								strbuff[13+apnbuff[0]+3+username[0]] = '"';
							}
							strbuff[13+apnbuff[0]+3+username[0]+1] = ',';
							strbuff[13+apnbuff[0]+3+username[0]+2] = '"';
							memcopy(strbuff+13+apnbuff[0]+3+username[0]+3,p,parlen);
							strbuff[13+apnbuff[0]+3+username[0]+3+parlen] = '"';
							strbuff[13+apnbuff[0]+3+username[0]+3+parlen+1] = '\r';
							strbuff[13+apnbuff[0]+3+username[0]+3+parlen+2] = '\0';
							
							EEPROM_OP((u8*)strbuff,APN_ADDR2,64,MODE_W);	
							result[resultindex] = 0;
							resultindex++;
						}
						else
						{
							result[resultindex] = 1;
							resultindex++;
						}
			
						break;						
					case 0x0017:	  //ip2
						if((parlen >= 7)&&(parlen <= 15))	//x.x.x.x
						{
							memcopy(ipbuff,p,parlen);
							ipbuff[parlen] =  0x0d;
							ipbuff[parlen+1] =	0x0a;
							if(SearchIPchar(ipbuff,ipstr,17))
							{
								EEPROM_OP((u8*)TCP_SERVER2,TCP_ADDR2,42,MODE_R);
								EEPROM_OP((u8*)UDP_SERVER2,UDP_ADDR2,42,MODE_R);
								memcopy(TCP_SERVER2+19,ipstr,15);//ip
								TCP_SERVER2[34]='"';
								memcopy(UDP_SERVER2+19,ipstr,15);//ip
								UDP_SERVER2[34]='"';
								UARTWrite((u8*)"+IP2set\r\n",9,DEBUG_COM);//debugggggg
								EEPROM_OP((u8*)TCP_SERVER2,TCP_ADDR2,35,MODE_W);
								EEPROM_OP((u8*)UDP_SERVER2,UDP_ADDR2,35,MODE_W);
											
								result[resultindex] = 0;
								resultindex++;
							}
							else
							{
								result[resultindex] = 1;
								resultindex++;
							}
						}
						else
						{
							result[resultindex] = 1;
							resultindex++;
						}
						break;
					case 0x0018:
						if(parlen == 4)   //ip1 tcpport
						{
							port = (*(p+3))+ ((u32)(*(p+2))<<8)+((u32)(*(p+1))<<16)+((u32)(*p)<<24);
							m = INT2Sry(port,portbuff);
							EEPROM_OP((u8*)TCP_SERVER,TCP_ADDR1,TCPCHARLEN,MODE_R);
							memcopy(TCP_SERVER+36,portbuff,m);//tcpport  2008
							UARTWrite((u8*)"+TCP1set\r\n",10,DEBUG_COM);//debugggggg
							TCP_SERVER[36+m]='\r';
							TCP_SERVER[36+m+1]='\0';
							EEPROM_OP((u8*)TCP_SERVER,TCP_ADDR1,TCPCHARLEN,MODE_W);
							result[resultindex] = 0;
							resultindex++;
						}
						else
						{
							result[resultindex] = 1;
							resultindex++;
						}
						break;	
					case 0x0019:
						if(parlen == 4)   //ip1 udpport
						{
							port = (*(p+3))+ ((u32)(*(p+2))<<8)+((u32)(*(p+1))<<16)+((u32)(*p)<<24);
							m = INT2Sry(port,portbuff);
							EEPROM_OP((u8*)UDP_SERVER,UDP_ADDR1,43,MODE_R);
							memcopy(UDP_SERVER+36,portbuff,m);//tcpport  2008
							UARTWrite((u8*)"+TCP2set\r\n",10,DEBUG_COM);//debugggggg
							UDP_SERVER[36+m]='\r';
							UDP_SERVER[36+m+1]='\0';
							EEPROM_OP((u8*)UDP_SERVER,UDP_ADDR1,43,MODE_W);
							result[resultindex] = 0;
							resultindex++;
						}
						else
						{
							result[resultindex] = 1;
							resultindex++;
						}
						break;
					case 0x0027:	  //定位时间间隔
						if(parlen == 4)
						{
							gpsUPTimeLEN =(*(p+3))+ ((u32)(*(p+2))<<8)+((u32)(*(p+1))<<16)+((u32)(*p)<<24);
							if(gpsUPTimeLEN >= 10) 
							{
								EEPROM_OP((u8*)&gpsUPTimeLEN,gpsUPTimeLEN_ADDR,4,MODE_W);
								result[resultindex] = 0;
								resultindex++;
							}
							else
							{
								result[resultindex] = 1;
								resultindex++;
							}
						}
						else
						{
							result[resultindex] = 1;
							resultindex++;
						}
						break;
					case 0x0029:	  //定位时间间隔
						if(parlen == 4)
						{
							gpsUPTimeLENsleep =(*(p+3))+ ((u32)(*(p+2))<<8)+((u32)(*(p+1))<<16)+((u32)(*p)<<24);
							if(gpsUPTimeLENsleep >= 10) 
							{
								EEPROM_OP((u8*)&gpsUPTimeLENsleep,gpsUPTimeLENsleep_ADDR,4,MODE_W);
								result[resultindex] = 0;
								resultindex++;
							}
							else
							{
								result[resultindex] = 1;
								resultindex++;
							}
						}
						else
						{
							result[resultindex] = 1;
							resultindex++;
						}
						break;
					case 0x0090:	  //定位模式
						if(parlen == 1)
						{
							result[resultindex] = 0;
							resultindex++;
						}
						else
						{
							result[resultindex] = 1;
							resultindex++;
						}
						break;
					
					case 0xF002:	  //域名
						if((parlen >0 )&&(parlen <=30))
						{
							str_cpy(DNS_ADDR,(u8*)"AT+QIDNSGIP=\"\0");
								
							memcopy(DNS_ADDR+13,p,parlen);
							DNS_ADDR[13+i]='"';
							DNS_ADDR[14+i]='\r';
							DNS_ADDR[15+i]='\0';
							EEPROM_OP((u8*)DNS_ADDR,DNS_ADDR1,str_len(DNS_ADDR)+1,MODE_W);
							UARTWrite((u8*)"+DNSset\r\n",9,DEBUG_COM);//debuggggggggggggggggggggggg						
							result[resultindex] = 0;
							resultindex++;
						}
						else
						{
							result[resultindex] = 1;
							resultindex++;
						}
						break;
					default:
						break;
				}
			}
			for(i = 0; i< resultindex;i++)
			{
				if(result[i] != 0)
					break;
			}
			if(i>= resultindex)
				DeviceAnswerdata.Result = 0;
			else
				DeviceAnswerdata.Result = 1;
			resultindex = 0;
			result[resultindex] = DeviceAnswerdata.AnswerServerSerialIndex[0];
			resultindex++;
			result[resultindex] = DeviceAnswerdata.AnswerServerSerialIndex[1];
			resultindex++;
			result[resultindex] = (u8)(((DeviceAnswerdata.AnswerServerMessageId)&0xff00)>>8);
			resultindex++;
			result[resultindex] = (u8)((DeviceAnswerdata.AnswerServerMessageId)&0xff);
			resultindex++;
			result[resultindex] = DeviceAnswerdata.Result;
			resultindex++;
			NewPacket_KY(result,resultindex,DEVICE_ANSWER,TCPLINK);
			break;
		case GET_DEVICE_SPEPARAM:	//查询指定参数
			index = 0; 
			j=0;
			resultindex = 0;			
			result[resultindex] = ServerMsgHead.SerialIndex[0];resultindex++;
			result[resultindex] = ServerMsgHead.SerialIndex[1];resultindex++;//应答流水号
			parnum = cmdbuf[12];	//参数总个数 
			result[resultindex] = 0;   //参数总个数 暂定为0
			resultindex++;
			for(i = 0;i < parnum;i++)
			{
				parID = cmdbuf[16+index]+ ((u32)cmdbuf[15+index]<<8)+((u32)cmdbuf[14+index]<<16)+((u32)cmdbuf[13+index]<<24);   //参数ID					
				switch(parID)
				{
					case 0x0001:	  //心跳时间间隔
					{
						j++;
						result[resultindex] = cmdbuf[13+index];resultindex++;
						result[resultindex] = cmdbuf[14+index];resultindex++;
						result[resultindex] = cmdbuf[15+index];resultindex++;
						result[resultindex] = cmdbuf[16+index];resultindex++;
						result[resultindex] = 4;resultindex++;
						result[resultindex] = (u8)(((DevParameters.HandTick)&0xff000000)>>24);			   
						resultindex++;
						result[resultindex] = (u8)(((DevParameters.HandTick)&0xff0000)>>16);
						resultindex++;
						result[resultindex] = (u8)(((DevParameters.HandTick)&0xff00)>>8);
						resultindex++;
						result[resultindex] = (u8)((DevParameters.HandTick)&0xff);
						resultindex++;						
						index+=4;
					}
						break;
					case 0x0002:	  //TCP重发时间间隔
					{
						j++;
						result[resultindex] = cmdbuf[13+index];resultindex++;
						result[resultindex] = cmdbuf[14+index];resultindex++;
						result[resultindex] = cmdbuf[15+index];resultindex++;
						result[resultindex] = cmdbuf[16+index];resultindex++;
						result[resultindex] = 4;resultindex++;
						result[resultindex] = (u8)(((DevParameters.GprsRetryTimeout)&0xff000000)>>24);			   
						resultindex++;
						result[resultindex] = (u8)(((DevParameters.GprsRetryTimeout)&0xff0000)>>16);
						resultindex++;
						result[resultindex] = (u8)(((DevParameters.GprsRetryTimeout)&0xff00)>>8);
						resultindex++;
						result[resultindex] = (u8)((DevParameters.GprsRetryTimeout)&0xff);
						resultindex++;						
						index+=4;
					}
						break;
					case 0x0003:	  //TCP重发次数	
					{						
						j++;
						result[resultindex] = cmdbuf[13+index];resultindex++;
						result[resultindex] = cmdbuf[14+index];resultindex++;
						result[resultindex] = cmdbuf[15+index];resultindex++;
						result[resultindex] = cmdbuf[16+index];resultindex++;
						result[resultindex] = 4;resultindex++;
						result[resultindex] = (u8)((DevParameters.GprsRetrycount&0xff000000)>>24);			   
						resultindex++;
						result[resultindex] = (u8)((DevParameters.GprsRetrycount&0xff0000)>>16);
						resultindex++;
						result[resultindex] = (u8)((DevParameters.GprsRetrycount&0xff00)>>8);
						resultindex++;
						result[resultindex] = (u8)(DevParameters.GprsRetrycount&0xff);
						resultindex++;						
						index+=4;
					}
						break;	
					case 0x0010:	  // APN 
					{
						j++;
						//AT+QICSGP=1,"CMNET","1234","567"
						for(k = 0 ;k<6;k++)
							comma[k] = 0;
						x = 0;
						apnbuff[0] = 0;
						EEPROM_OP((u8*)strbuff,APN_ADDR1,64,MODE_R);
						for(k = 0;i<64;k++)
						{
							if(strbuff[k] == 0x22)
							{
								comma[x] = k;
								x++;
							}
							if(strbuff[k] == '\r')
							{
								break;
							}
						}
						if((comma[1] - comma[0]) > 0)
						{
							apnbuff[0] = (comma[1] - comma[0]-1);
							memcopy(apnbuff+1,strbuff+comma[0]+1,apnbuff[0]);
						}						
						result[resultindex] = cmdbuf[13+index];resultindex++;
						result[resultindex] = cmdbuf[14+index];resultindex++;
						result[resultindex] = cmdbuf[15+index];resultindex++;
						result[resultindex] = cmdbuf[16+index];resultindex++;
						result[resultindex] = apnbuff[0];resultindex++;
						memcopy(result+resultindex,apnbuff+1,apnbuff[0]);
						resultindex += apnbuff[0];
						index+=4;
					}
						break;	
					case 0x0011:	  // APN--username		
					{						
						j++;
						//AT+QICSGP=1,"CMNET","1234","567"
						for(k = 0 ;k<6;k++)
							comma[k] = 0;
						x = 0;
						username[0] = 0;
						EEPROM_OP((u8*)strbuff,APN_ADDR1,64,MODE_R);
						for(k = 0;i<64;k++)
						{
							if(strbuff[k] == 0x22)
							{
								comma[x] = k;
								x++;
							}
							if(strbuff[k] == '\r')
							{
								break;
							}
						}
						if((comma[3] - comma[2]) > 0)
						{
							username[0] = (comma[3] - comma[2]-1);
							memcopy(username+1,strbuff+comma[2]+1,username[0]);
						}
						result[resultindex] = cmdbuf[13+index];resultindex++;
						result[resultindex] = cmdbuf[14+index];resultindex++;
						result[resultindex] = cmdbuf[15+index];resultindex++;
						result[resultindex] = cmdbuf[16+index];resultindex++;
						result[resultindex] = apnbuff[0];resultindex++;
						memcopy(result+resultindex,username+1,username[0]);
						resultindex += username[0];
						index+=4;
					}
						break;	
					case 0x0012:	  // APN--password
					{
						j++;
						//AT+QICSGP=1,"CMNET","1234","567"
						for(k = 0 ;k<6;k++)
							comma[k] = 0;
						x = 0;
						password[0] = 0;
						EEPROM_OP((u8*)strbuff,APN_ADDR1,64,MODE_R);
						for(k = 0;i<64;k++)
						{
							if(strbuff[k] == 0x22)
							{
								comma[x] = k;
								x++;
							}
							if(strbuff[k] == '\r')
							{
								break;
							}
						}
						if((comma[5] - comma[4]) > 0)
						{
							password[0] = (comma[5] - comma[4]-1);
							memcopy(password+1,strbuff+comma[4]+1,password[0]);
						}
						result[resultindex] = cmdbuf[13+index];resultindex++;
						result[resultindex] = cmdbuf[14+index];resultindex++;
						result[resultindex] = cmdbuf[15+index];resultindex++;
						result[resultindex] = cmdbuf[16+index];resultindex++;
						result[resultindex] = apnbuff[0];resultindex++;
						memcopy(result+resultindex,password+1,password[0]);
						resultindex += password[0];
						index+=4;
						break;			
					}
					case 0x0013:	 //IP1
					{
						j++;
						EEPROM_OP((u8*)strbuff,TCP_ADDR1,TCPCHARLEN,MODE_R);
						result[resultindex] = cmdbuf[13+index];resultindex++;
						result[resultindex] = cmdbuf[14+index];resultindex++;
						result[resultindex] = cmdbuf[15+index];resultindex++;
						result[resultindex] = cmdbuf[16+index];resultindex++;
						result[resultindex] = 15;resultindex++;
						memcopy(result+resultindex,strbuff+19,15);
						resultindex += 15;
						index+=4;			
					}						
						break;
					case 0x0014:	  // APN2 
					{
						j++;
						//AT+QICSGP=1,"CMNET","1234","567"
						for(k = 0 ;k<6;k++)
							comma[k] = 0;
						x = 0;
						apnbuff[0] = 0;
						EEPROM_OP((u8*)strbuff,APN_ADDR2,64,MODE_R);
						for(k = 0;i<64;k++)
						{
							if(strbuff[k] == 0x22)
							{
								comma[x] = k;
								x++;
							}
							if(strbuff[k] == '\r')
							{
								break;
							}
						}
						if((comma[1] - comma[0]) > 0)
						{
							apnbuff[0] = (comma[1] - comma[0]-1);
							memcopy(apnbuff+1,strbuff+comma[0]+1,apnbuff[0]);
						}						
						result[resultindex] = cmdbuf[13+index];resultindex++;
						result[resultindex] = cmdbuf[14+index];resultindex++;
						result[resultindex] = cmdbuf[15+index];resultindex++;
						result[resultindex] = cmdbuf[16+index];resultindex++;
						result[resultindex] = apnbuff[0];resultindex++;
						memcopy(result+resultindex,apnbuff+1,apnbuff[0]);
						resultindex += apnbuff[0];
						index+=4;			
					}						
						break;	
					case 0x0015:	  // APN2--username
					{
						j++;
						//AT+QICSGP=1,"CMNET","1234","567"
						for(k = 0 ;k<6;k++)
							comma[k] = 0;
						x = 0;
						username[0] = 0;
						EEPROM_OP((u8*)strbuff,APN_ADDR2,64,MODE_R);
						for(k = 0;i<64;k++)
						{
							if(strbuff[k] == 0x22)
							{
								comma[x] = k;
								x++;
							}
							if(strbuff[k] == '\r')
							{
								break;
							}
						}
						if((comma[3] - comma[2]) > 0)
						{
							username[0] = (comma[3] - comma[2]-1);
							memcopy(username+1,strbuff+comma[2]+1,username[0]);
						}
						result[resultindex] = cmdbuf[13+index];resultindex++;
						result[resultindex] = cmdbuf[14+index];resultindex++;
						result[resultindex] = cmdbuf[15+index];resultindex++;
						result[resultindex] = cmdbuf[16+index];resultindex++;
						result[resultindex] = username[0];resultindex++;
						memcopy(result+resultindex,username+1,username[0]);
						resultindex += username[0];
						index+=4;		
					}						
						break;	
					case 0x0016:	  // APN--password
					{
						j++;
						//AT+QICSGP=1,"CMNET","1234","567"
						for(k = 0 ;k<6;k++)
							comma[k] = 0;
						x = 0;
						password[0] = 0;
						EEPROM_OP((u8*)strbuff,APN_ADDR2,64,MODE_R);
						for(k = 0;i<64;k++)
						{
							if(strbuff[k] == 0x22)
							{
								comma[x] = k;
								x++;
							}
							if(strbuff[k] == '\r')
							{
								break;
							}
						}
						if((comma[5] - comma[4]) > 0)
						{
							password[0] = (comma[5] - comma[4]-1);
							memcopy(password+1,strbuff+comma[4]+1,password[0]);
						}
						result[resultindex] = cmdbuf[13+index];resultindex++;
						result[resultindex] = cmdbuf[14+index];resultindex++;
						result[resultindex] = cmdbuf[15+index];resultindex++;
						result[resultindex] = cmdbuf[16+index];resultindex++;
						result[resultindex] = password[0];resultindex++;
						memcopy(result+resultindex,password+1,password[0]);
						resultindex += password[0];	
						index+=4;		
					}
						break;						
					case 0x0017:	  //ip2
					{
						j++;
						EEPROM_OP((u8*)strbuff,TCP_ADDR2,TCPCHARLEN,MODE_R);
						result[resultindex] = cmdbuf[13+index];resultindex++;
						result[resultindex] = cmdbuf[14+index];resultindex++;
						result[resultindex] = cmdbuf[15+index];resultindex++;
						result[resultindex] = cmdbuf[16+index];resultindex++;
						result[resultindex] = 15;resultindex++;
						memcopy(result+resultindex,strbuff+19,15);
						resultindex += 15;

						index+=4;				
					}						
						break;
					case 0x0018:
					{
						j++;
						portbuff[0] = 0;
						//AT+QIOPEN=0,"TCP","058.241.040.148",5088\r\0 
						EEPROM_OP((u8*)strbuff,TCP_ADDR1,TCPCHARLEN,MODE_R);
						y = str_len(strbuff);
						if(y > (36+2))
						{
							portbuff[0] = y-38;
							memcopy(portbuff+1,strbuff+36,portbuff[0]);//tcpport  2008
						}
						if(portbuff[0])
						{
							result[resultindex] = cmdbuf[13+index];resultindex++;
							result[resultindex] = cmdbuf[14+index];resultindex++;
							result[resultindex] = cmdbuf[15+index];resultindex++;
							result[resultindex] = cmdbuf[16+index];resultindex++;
							result[resultindex] = portbuff[0];resultindex++;
							memcopy(result+resultindex,portbuff+1,portbuff[0]);
							resultindex += portbuff[0];
						}
						index+=4;		
					}						
						break;	
					case 0x0019:
					{
						j++;
						portbuff[0] = 0;
						//AT+QIOPEN=0,"UDP","058.241.040.148",5089\r\0 
						EEPROM_OP((u8*)strbuff,UDP_ADDR1,TCPCHARLEN,MODE_R);
						y = str_len(strbuff);
						if(y > (36+2))
						{
							portbuff[0] = y-38;
							memcopy(portbuff+1,strbuff+36,portbuff[0]);//tcpport  2008
						}
						if(portbuff[0])
						{
							result[resultindex] = cmdbuf[13+index];resultindex++;
							result[resultindex] = cmdbuf[14+index];resultindex++;
							result[resultindex] = cmdbuf[15+index];resultindex++;
							result[resultindex] = cmdbuf[16+index];resultindex++;
							result[resultindex] = portbuff[0];resultindex++;
							memcopy(result+resultindex,portbuff+1,portbuff[0]);
							resultindex += portbuff[0];
						}
						index+=4;		
					}						
						break;
					case 0x0027:	  
					{
						j++;
						result[resultindex] = cmdbuf[13+index];resultindex++;
						result[resultindex] = cmdbuf[14+index];resultindex++;
						result[resultindex] = cmdbuf[15+index];resultindex++;
						result[resultindex] = cmdbuf[16+index];resultindex++;
						result[resultindex] = 4;resultindex++;
						result[resultindex] = (u8)((gpsUPTimeLEN&0xff000000)>>24);			   
						resultindex++;
						result[resultindex] = (u8)((gpsUPTimeLEN&0xff0000)>>16);
						resultindex++;
						result[resultindex] = (u8)((gpsUPTimeLEN&0xff00)>>8);
						resultindex++;
						result[resultindex] = (u8)(gpsUPTimeLEN&0xff);
						resultindex++;
						index+=4;		
					}
						break;
					case 0x0029:	  //定位时间间隔
					{
						j++;
						result[resultindex] = cmdbuf[13+index];resultindex++;
						result[resultindex] = cmdbuf[14+index];resultindex++;
						result[resultindex] = cmdbuf[15+index];resultindex++;
						result[resultindex] = cmdbuf[16+index];resultindex++;
						result[resultindex] = 4;resultindex++;
						result[resultindex] = (u8)((gpsUPTimeLENsleep&0xff000000)>>24);			   
						resultindex++;
						result[resultindex] = (u8)((gpsUPTimeLENsleep&0xff0000)>>16);
						resultindex++;
						result[resultindex] = (u8)((gpsUPTimeLENsleep&0xff00)>>8);
						resultindex++;
						result[resultindex] = (u8)(gpsUPTimeLENsleep&0xff);
						resultindex++;
						index+=4;		
					}
						break;
					case 0x0090:	  //定位模式
					{
						j++;
						result[resultindex] = cmdbuf[13+index];resultindex++;
						result[resultindex] = cmdbuf[14+index];resultindex++;
						result[resultindex] = cmdbuf[15+index];resultindex++;
						result[resultindex] = cmdbuf[16+index];resultindex++;
						result[resultindex] = 1;resultindex++;
						result[resultindex] = 3;resultindex++;
						index+=4;
					}
						break;
					case 0xF000:	  //查询锁状态
					{
						j++;
						result[resultindex] = cmdbuf[13+index];resultindex++;//参数ID号
						result[resultindex] = cmdbuf[14+index];resultindex++;//参数ID号
						result[resultindex] = cmdbuf[15+index];resultindex++;//参数ID号
						result[resultindex] = cmdbuf[16+index];resultindex++;//参数ID号
						result[resultindex] = 4;resultindex++;
						result[resultindex] = LockStatus;resultindex++;
						result[resultindex] = (Adc_Vdd>>8);resultindex++;		
						result[resultindex] = (Adc_Vdd&0xff);resultindex++;
						result[resultindex] = ((TT_Alarm_Status&ayjks)?(1):(0))*8+((TT_Alarm_Status& aLockCUT)?(1):(0))*4+((TT_Alarm_Status& aLockOPEN)?(1):(0))*2+ ((TT_Alarm_Status& aVDDLOW)?(1):(0));
						resultindex++;
						index+=4;
					}
						break;
					case 0xF001:	  
					{
						j++;
						result[resultindex] = cmdbuf[13+index];resultindex++;
						result[resultindex] = cmdbuf[14+index];resultindex++;
						result[resultindex] = cmdbuf[15+index];resultindex++;
						result[resultindex] = cmdbuf[16+index];resultindex++;
						result[resultindex] = (8*MAXBC_NUM);resultindex++;
						///////添加锁操作记录
						for(codeindex = 0;codeindex<MAXBC_NUM;codeindex++)
						{
							result[resultindex] = (MAXBC_NUM-codeindex);resultindex++;
							EEPROM_OP(tmp,((u32)(LockNEWEVENT_ADDR)+((u32)(RecordIndex+codeindex-1)% MAXBC_NUM)*NEWRECORDDATALEN),NEWRECORDDATALEN,MODE_R);
							memcopy(result+resultindex,tmp+1,7);
							resultindex+=7;
						}
						result[resultindex] = 0;resultindex++;
						index+=4;
					}
						break;
					case 0xF002:	  //域名
					{
						j++;
						EEPROM_OP((u8*)strbuff,DNS_ADDR1,str_len(DNS_ADDR)+1,MODE_R);
						result[resultindex] = cmdbuf[13+index];resultindex++;
						result[resultindex] = cmdbuf[14+index];resultindex++;
						result[resultindex] = cmdbuf[15+index];resultindex++;
						result[resultindex] = cmdbuf[16+index];resultindex++;
						result[resultindex] = str_len(DNS_ADDR)+1;resultindex++;
						memcopy(result+resultindex,strbuff+13,str_len(DNS_ADDR)+1-13);
						resultindex += str_len(DNS_ADDR)+1-13;
						index+=4;
					}
						break;
					default:
						index+=4;	
						break;						
				}
			}
			result[2] = j;
			NewPacket_KY(result,resultindex,GET_DEVICE_PARAM_ANSWER,TCPLINK);	//设备应答
			break;
		case CHECK_LOCATION:			
			resultindex = 0;
			result[resultindex] = ServerMsgHead.SerialIndex[0];
			resultindex++;
			result[resultindex] = ServerMsgHead.SerialIndex[1];
			resultindex++;
			//添加定位数据
			GpsLocationOperate(&gpsx,result,CHECK_LOCATION_ANSWER,TCPLINK);
			break;

		case DEVICE_UNLOCK:
		case DEVICE_LOCK:
		case DEVICE_CLRALARM:
			resultindex = 0;
			result[resultindex] = ServerMsgHead.SerialIndex[0];resultindex++;
			result[resultindex] = ServerMsgHead.SerialIndex[1];resultindex++;

			CenterCmdOP((u8*)(&cmdbuf[12]),msglen,result,ServerMsgHead.MessageId,SERVER_OPR);
			break;
		default:
			UARTWrite(cmdbuf,cmdbuf[3],DEBUG_COM);
			break;
		}
	}
}


/*******************************************************************************
* Function Name  : IPset
* Description    : 存储IP，更改IP
* Input          :  
* Return         :  
*******************************************************************************/
u8 IPset(u8 cmd,u8* src,u8 Type)
{
	u8	i;

	if(cmd == 0x30)//IPreset//
	{
		str_cpy(DNS_ADDR,(u8*)DNS_ADDR0);
		str_cpy(TCP_SERVER,(u8*)TCP_SERVER0);
		str_cpy(UDP_SERVER,(u8*)UDP_SERVER0);
		str_cpy(APN_PARA,(u8*)APN_PARA0);
		str_cpy(TCP_PORT,(u8*)TCP_PORT0);
		str_cpy(UDP_PORT,(u8*)UDP_PORT0);

		EEPROM_OP((u8*)DNS_ADDR,DNS_ADDR1,str_len(DNS_ADDR)+1,MODE_W);
		EEPROM_OP((u8*)TCP_SERVER,TCP_ADDR1,str_len(TCP_SERVER)+1,MODE_W);
		EEPROM_OP((u8*)UDP_SERVER,UDP_ADDR1,str_len(UDP_SERVER)+1,MODE_W);
		EEPROM_OP((u8*)APN_PARA,APN_ADDR1,str_len(APN_PARA)+1,MODE_W);
		EEPROM_OP((u8*)TCP_PORT,TCP_PORT_ADDR1,str_len(TCP_PORT)+1,MODE_W);
		EEPROM_OP((u8*)UDP_PORT,UDP_PORT_ADDR1,str_len(UDP_PORT)+1,MODE_W);  

		str_cpy(TCP_SERVER2,(u8*)TCP_SERVER3);
		str_cpy(UDP_SERVER2,(u8*)UDP_SERVER3);
		str_cpy(APN_PARA2,(u8*)APN_PARA3);

		EEPROM_OP((u8*)TCP_SERVER2,TCP_ADDR2,str_len(TCP_SERVER2)+1,MODE_W);
		EEPROM_OP((u8*)UDP_SERVER2,UDP_ADDR2,str_len(UDP_SERVER2)+1,MODE_W);
		EEPROM_OP((u8*)APN_PARA2,APN_ADDR2,str_len(APN_PARA2)+1,MODE_W);
		return 2;
	}
 	else if(cmd == 0x00)
	{
		if((src[4]>0x2F)&&(src[4]<0x3A))//0-9//
		{
			memcopy(TCP_PORT,src+4,4);
			TCP_PORT[4] = '\r';
			TCP_PORT[5] = '\0';
			memcopy(UDP_PORT,src+8,4);	 
			UDP_PORT[4] = '\r';
			UDP_PORT[5] = '\0';
		
			EEPROM_OP((u8*)TCP_PORT,TCP_PORT_ADDR1,6,MODE_W);
			EEPROM_OP((u8*)UDP_PORT,UDP_PORT_ADDR1,6,MODE_W);  
			return 2;
		}
		else
		{
			return 0;
		}
	}
 	else if(cmd == 0x01)//IP1//
	{
		//059.041.073.034,2008,2009,CMNET....//
		if((Type == DEBUG_COM)||(Type == SMSLINK))
			i = src[3]-src[2]-4-25;
		else
			i = src[3]-src[2]-4-25-1;
		memcopy((u8*)TCP_SERVER,(u8*)TCP_SERVER0,19);
		memcopy((u8*)UDP_SERVER,(u8*)UDP_SERVER0,19);
		if(((src[0 +1+3]>0x2F)&&(src[0+1+3]<0x33))
			&&((src[16+1+3]>0x2F)&&(src[16+1+3]<0x3A))
			&&((src[21+1+3]>0x2F)&&(src[21+1+3]<0x3A)))
		{
			memcopy(TCP_SERVER+19,src+1+3,15);//ip
			TCP_SERVER[34]='"';
			memcopy(UDP_SERVER+19,src+1+3,15);//ip
			UDP_SERVER[34]='"';
			UARTWrite((u8*)"+IP1set\r\n",9,DEBUG_COM);//debugggggg


			memcopy(TCP_SERVER+35,src+15+1+3,5);//tcpport ,2008
			UARTWrite((u8*)"+TCP1set\r\n",10,DEBUG_COM);//debugggggg

			TCP_SERVER[40]='\r';
			TCP_SERVER[41]='\0';
			EEPROM_OP((u8*)TCP_SERVER,TCP_ADDR1,42,MODE_W);

			memcopy(UDP_SERVER+35,src+20+1+3,5);//UDPPort1 ,2009
			UARTWrite((u8*)"+UDP1set\r\n",10,DEBUG_COM);//debugggggg

			UDP_SERVER[40]='\r';
			UDP_SERVER[41]='\0';
			EEPROM_OP((u8*)UDP_SERVER,UDP_ADDR1,42,MODE_W);
			if(i == 0)
			{
				return 2;
			}
			else
			{
				//APN://
				if((src[25+1+3]==',')&&((src[26+1+3]>0x20)&&(src[26+1+3]<0x7F)))
				{
					if(i<= 46)	  //APN字符最长40字符
					{
						str_cpy(APN_PARA,(u8*)"AT+QICSGP=1,\"\0");
						str_cpy(APN_PARA+13,(u8*)(src+26+1+3));
						APN_PARA[13+i-1]='"';
						APN_PARA[14+i-1]='\r';
						APN_PARA[15+i-1]='\0';
						EEPROM_OP((u8*)APN_PARA,APN_ADDR1,str_len(APN_PARA)+1,MODE_W);
						UARTWrite((u8*)"+APN1set\r\n",10,DEBUG_COM);//debuggggggggggggggggggggggg
						return 2;
					}
					else
					{
						return 0;
					}
				}
				else
				{
					return 0;
				}
			}
		}
		else
		{
			return 0;
		}
	}
 	else if(cmd == 0x02)//IP2//
	{
		//059.041.073.034,2008,2009,CMNET....//
		if((Type == DEBUG_COM)||(Type == SMSLINK))
			i = src[3]-src[2]-4-25;
		else
			i = src[3]-src[2]-4-25-1;
		memcopy((u8*)TCP_SERVER2,(u8*)TCP_SERVER3,19);
		memcopy((u8*)UDP_SERVER2,(u8*)UDP_SERVER3,19);
		if(((src[0+1+3]>0x2F)&&(src[0+1+3]<0x33))
			&&((src[16+1+3]>0x2F)&&(src[16+1+3]<0x3A))
			&&((src[21+1+3]>0x2F)&&(src[21+1+3]<0x3A)))
		{
			memcopy(TCP_SERVER2+19,src+1+3,15);//ip
			TCP_SERVER2[34]='"';

			memcopy(UDP_SERVER2+19,src+1+3,15);//ip
			UDP_SERVER2[34]='"';
			UARTWrite((u8*)"+IP2set\r\n",9,DEBUG_COM);//debugggggg


			memcopy(TCP_SERVER2+35,src+15+1+3,5);//tcpport ,2008
			UARTWrite((u8*)"+TCP2set\r\n",10,DEBUG_COM);//debugggggg

			TCP_SERVER2[40]='\r';
			TCP_SERVER2[41]='\0';
			EEPROM_OP((u8*)TCP_SERVER2,TCP_ADDR2,42,MODE_W);

			memcopy(UDP_SERVER2+35,src+20+1+3,5);//UDPPort1 ,2009
			UARTWrite((u8*)"+UDP2set\r\n",10,DEBUG_COM);//debugggggg

			UDP_SERVER2[40]='\r';
			UDP_SERVER2[41]='\0';
			EEPROM_OP((u8*)UDP_SERVER2,UDP_ADDR2,42,MODE_W);
			if(i == 0)
			{
				return 2;
			}
			else
			{
				//APN://
				if((src[25+1+3]==',')&&((src[26+1+3]>0x20)&&(src[26+1+3]<0x7F)))
				{
					if(i<=46)							//APN字符最长40字符
					{
						str_cpy(APN_PARA2,(u8*)"AT+QICSGP=1,\"\0");
						str_cpy(APN_PARA2+13,(u8*)(src+26+1+3));
						APN_PARA2[13+i-1]='"';
						APN_PARA2[14+i-1]='\r';
						APN_PARA2[15+i-1]='\0';
						EEPROM_OP((u8*)APN_PARA2,APN_ADDR2,str_len(APN_PARA2)+1,MODE_W);
						UARTWrite((u8*)"+APN2set\r\n",10,DEBUG_COM);//debuggggggggggggggggggggggg
							return 2;
					}
					else
					{
						return 0;
					}
				}
				else
				{
					return 0;
				}
			}
		}
		else
		{
			return 0;
		}
	}
 	else if(cmd == 0x03)//DNS//
	{
		if((src[4]>0x20)&&(src[4]<0x7F))//ASCII//
		{
			//i=str_len((u8*)(src+4));
			if(Type == DEBUG_COM)
				i = src[3]-src[2]-4;
			else
				i = src[3]-src[2]-4-1;
			
			if(i<=(47-17))			//DNS字符最长30字符
			{
				str_cpy(DNS_ADDR,(u8*)"AT+QIDNSGIP=\"\0");
				str_cpy(DNS_ADDR+13,(u8*)(src+4));
				//i=str_len((u8*)(src+0));
				DNS_ADDR[13+i]='"';
				DNS_ADDR[14+i]='\r';
				DNS_ADDR[15+i]='\0';
				EEPROM_OP((u8*)DNS_ADDR,DNS_ADDR1,str_len(DNS_ADDR)+1,MODE_W);
				UARTWrite((u8*)"+DNSset\r\n",9,DEBUG_COM);//debuggggggggggggggggggggggg
				return 2;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			return 0;
		}
	}
	else 
		return 2;
}
/*******************************************************************************
* Function Name  : TransferReceiveData
* Description    : 转义
* Input          :  
* Return         :  
*******************************************************************************/
u16 TransferReceiveData(u8* abuf,u16 len)
{
	u16 hexpt,i;

	hexpt = len;
	for(i = 0;i < (hexpt-1);i++)
	{
		if((abuf[i] == 0x7D)&&(abuf[i+1]==0x01))
		{	
			if((i+2) < hexpt)
			{
				memcopy(abuf+i+1,abuf+i+2,(hexpt-i-2));
				hexpt--;
			}
			else if((i+2) == hexpt)
				hexpt--;
		}
		else if((abuf[i] == 0x7D)&&(abuf[i+1]==0x02))
		{
			abuf[i] = 0x7E;
			if((i+2) < hexpt)
			{
				memcopy(abuf+i+1,abuf+i+2,(hexpt-i-2));
				hexpt--;
			}
			else if((i+2) == hexpt)
				hexpt--;
		}
	}	
	return hexpt;
}
/*******************************************************************************
* Function Name  : TransferSendData
* Description    : 收到的时候  转义
* Input          :  
* Return         :  
*******************************************************************************/
u16 TransferSendData(u8* abuf,u16 len)
{
	u16 hexpt,i,j;
	u8 hbuf[1024];

	if(len <= 2)
		return 0;
	if((abuf[0] != 0x7E)||(abuf[len-1] != 0x7E))
		return 0;
	hexpt = len - 2;
	memcopy(hbuf,abuf+1,hexpt);
	
	for(i = 0;i < (hexpt);i++)
	{
		if(hbuf[i] == 0x7D)
		{	
			for(j= 0;j<(hexpt -i);j++)
				hbuf[hexpt-j] = hbuf[hexpt -j-1];
      hbuf[i+1] = 0x01;
			hexpt++;
            
		}
		else if(hbuf[i] == 0x7E)
		{	
			for(j= 0;j<(hexpt -i);j++)
				hbuf[hexpt-j] = hbuf[hexpt -j-1];
			hbuf[i] = 0x7D;
			hbuf[i+1] = 0x02;
			hexpt++;
		}
	}	
	abuf[0] = 0x7E;	
	memcopy(abuf+1,hbuf,hexpt);
	abuf[hexpt+1] = 0x7E;	
	hexpt += 2;
	
	return hexpt;
}
/*******************************************************************************
* Function Name  : DataFenBao
* Description    : 收到后台的数据  然后解析
* Input          :  
* Return         :  
*******************************************************************************/
void DataFenBao(u8 * abuf,u8 asclen,u8 link)
{
	u8 hbuf[512],hexpt,i,crcor;

	if(asclen>253)
	{
		return;
	}
	memcopy(hbuf,abuf,asclen);
	
	hexpt = TransferReceiveData(hbuf,asclen);  //转义解析...
	//SendData2TestCOM(hbuf,hexpt,0);

	if(link==DEBUG_COM||(link==SMSLINK))
	{
		hexpt=ASCII2HEX(abuf,asclen,hbuf);
	}
	else
	{
		i = hbuf[hexpt-1]&0xff;
		crcor = Do_XOR(hbuf,hexpt-1);
		if(i == crcor)
		{
			RcvDataOP(hbuf,hexpt,link);
		}
		else
			UARTWrite((u8 *)"CRCerr\r\n",8,DEBUG_COM);
	}
}


