//���ĵ���������3����λ����ͨѶЭ��
//�������ƶ��壺Serial3_��������;
#include "serial.h"
#include "common.h"
#include "config.h"
#include "global.h"

_set_lock_para lock_para;
uint8_t Result = 0;//���
uint8_t Ack_State = 0;

extern u8 Store_LockPara[100];//27+25*3=102�ֽڣ�����������Ϣ

int Serial3_RxCB(void)
{
	uint8_t crcResult,temp_crcResult;
	uint16_t rx_len;
	//��������У��
	if ((Rx3Buf[0] == 0xA5) && (Rx3Buf[1] == 0xA5))
	{
			rx_len = Rx3Buf[2];
			rx_len = ((rx_len<<8)&0xff00) | Rx3Buf[3];//ȡ������
		if(Rx1Num==rx_len)
		{
			crcResult = Rx3Buf[rx_len-1];
			temp_crcResult = Do_XOR(Rx3Buf,rx_len-1);//����CRC
			if(crcResult == temp_crcResult)
			{
				switch(Rx3Buf[4])
				{
					case CMD_WRITE_SN: //д��SN��
						Serial3_Write_SN();
						return WRITE_SN_OK;
					case CMD_SET_LOCK: //����������
						Serial3_SET_LOCK();
						return SET_LOCK_OK;
					case CMD_QUERY_LOCK: //��ѯ������
						Serial3_QUERY_LOCK();
						return QUERY_LOCK_OK;
					default:break;
				}
			}
			else
				 return FRM_CRC_ERROR;//crc����
		}
		else
				return FRM_LEN_ERROR;//���ȴ���
	}
	else
			return FRM_HEADD_ERROR;//֡ͷ����
	return 0;
}
//д��SN��
void Serial3_Write_SN(void)
{
	uint8_t i;
	u8 temp_buf1[6],temp_buf2[6];

	for (i=0;i<6;i++)
		lock_para.lock_sn[i]=Rx3Buf[5+i];
	for (i=0;i<10;i++)
		lock_para.reserve[i]=Rx3Buf[11+i];

	//д��SN
	ee_WriteBytes(lock_para.lock_sn, LOCKSN_ADDR1, 6);
	ee_WriteBytes(lock_para.lock_sn, LOCKSN_ADDR2, 6);
	//����SN
	ee_ReadBytes(temp_buf1, LOCKSN_ADDR1, 6);
	ee_ReadBytes(temp_buf2, LOCKSN_ADDR2, 6);
	
	if(Buffercmp(lock_para.lock_sn,temp_buf1,6)&Buffercmp(lock_para.lock_sn,temp_buf2,6))
	{Result=RES_SUCCESS;}
	else
	{Result=RES_COMP_FAIL;}
}
//Ӧ��д��SN��
void ACK_Serial3_Write_SN(uint8_t command,uint8_t state)
{
	u8 temp_tmpbuf[30];
	uint16_t length;
	u8 temp_crc;
	uint16_t hexpt=0,temp_hexpt=0;

	hexpt=0;
	temp_tmpbuf[hexpt] = 0xA5;	hexpt++;														//hexpt=1
	temp_tmpbuf[hexpt] = 0xA5;	hexpt++;														//hexpt=2
  hexpt+=2; temp_hexpt = 	hexpt;																	//hexpt=4
	temp_tmpbuf[hexpt] = command;	hexpt++;													//hexpt=5
	
	memcopy(temp_tmpbuf+hexpt,lock_para.lock_sn,sizeof_lock_sn);
	hexpt+=sizeof_lock_sn;																					//hexpt=11
	temp_tmpbuf[hexpt] = state;	hexpt++;                            //hexpt=12
	memset(temp_tmpbuf+hexpt,0xFF,sizeof_reserve);
	hexpt+=sizeof_reserve;																					//hexpt=22
	//���㳤��
	length = hexpt;
	//�������֡����
 	temp_tmpbuf[temp_hexpt-2] = length / 256;
 	temp_tmpbuf[temp_hexpt-1] = length & 0xFF;
	//����CRC
	temp_crc = Do_XOR(temp_tmpbuf,length);
	temp_tmpbuf[hexpt] = temp_crc;	hexpt++;          							//hexpt=23
	//����Ӧ������֡
	Usart_SendArray(USART3, temp_tmpbuf, hexpt);
}
//���SN���Ƿ�ƥ��
static u8 Check_SN(void)
{
	u8 i;
	u8 temp_buf[6];
	
	for (i=0;i<6;i++)
		lock_para.lock_sn[i]=Rx3Buf[5+i];
	//����SN
	ee_ReadBytes(temp_buf, LOCKSN_ADDR1, 6);
	//�Ƚ�SN�Բ���
	return Buffercmp(lock_para.lock_sn,temp_buf,6);
}
//����������
void Serial3_SET_LOCK(void)
{
	u8 temp_buf[100];
	u8 len;
	
	if(ERROR == Check_SN())
	{
		Result=RES_SN_ERROR;
	}
	else
	{
		lock_para.heartbeat_gap=Rx3Buf[11]<<24|Rx3Buf[12]<<16|Rx3Buf[13]<<8|Rx3Buf[14];
		lock_para.TCP_timeout=Rx3Buf[15]<<24|Rx3Buf[16]<<16|Rx3Buf[17]<<8|Rx3Buf[18];
		lock_para.gps_gap_sleep=Rx3Buf[19]<<24|Rx3Buf[20]<<16|Rx3Buf[21]<<8|Rx3Buf[22];
		lock_para.gps_gap_active=Rx3Buf[23]<<24|Rx3Buf[24]<<16|Rx3Buf[25]<<8|Rx3Buf[26];
		lock_para.arc_corner=Rx3Buf[27];
		lock_para.gnss_mode=Rx3Buf[28];
		
		lock_para.server1_addr_len=Rx3Buf[29];
		memcopy((unsigned char *)lock_para.server1_addr,Rx3Buf+30,lock_para.server1_addr_len);		//������1��ַ
		lock_para.server1_port=Rx3Buf[30+lock_para.server1_addr_len]<<8|Rx3Buf[30+lock_para.server1_addr_len+1];
		
		lock_para.server2_addr_len=Rx3Buf[32+lock_para.server1_addr_len];
		memcopy((unsigned char *)lock_para.server2_addr,Rx3Buf+33+lock_para.server1_addr_len,lock_para.server2_addr_len);		//������1��ַ
		lock_para.server2_port=Rx3Buf[33+lock_para.server1_addr_len+lock_para.server2_addr_len]<<8|Rx3Buf[33+lock_para.server1_addr_len+lock_para.server2_addr_len+1];
		
		lock_para.server3_addr_len=Rx3Buf[35+lock_para.server1_addr_len+lock_para.server2_addr_len];
		memcopy((unsigned char *)lock_para.server3_addr,Rx3Buf+36+lock_para.server1_addr_len+lock_para.server2_addr_len,lock_para.server3_addr_len);		//������1��ַ
		lock_para.server3_port=Rx3Buf[36+lock_para.server1_addr_len+lock_para.server2_addr_len+lock_para.server3_addr_len]<<8|Rx3Buf[36+lock_para.server1_addr_len+lock_para.server2_addr_len+lock_para.server3_addr_len+1];
		
		memset(lock_para.reserve,0xFF,10);
		//�������洢BUF���棬���ڴ洢FLASH
		memcopy(Store_LockPara, Rx3Buf+11, 4);
		memcopy(Store_LockPara+4, Rx3Buf+15, 4);
		memcopy(Store_LockPara+8, Rx3Buf+19, 4);
		memcopy(Store_LockPara+12, Rx3Buf+23, 4);
		memcopy(Store_LockPara+16, Rx3Buf+27, 1);
		memcopy(Store_LockPara+17, Rx3Buf+28, 1);
		
		memcopy(Store_LockPara+18, Rx3Buf+29, 1);
		memcopy(Store_LockPara+19, Rx3Buf+32+lock_para.server1_addr_len, 1);
		memcopy(Store_LockPara+20, Rx3Buf+35+lock_para.server1_addr_len+lock_para.server2_addr_len, 1);
		
		memcopy(Store_LockPara+21, Rx3Buf+30+lock_para.server1_addr_len, 2);
		memcopy(Store_LockPara+23, Rx3Buf+33+lock_para.server1_addr_len+lock_para.server2_addr_len, 2);
		memcopy(Store_LockPara+25, Rx3Buf+36+lock_para.server1_addr_len+lock_para.server2_addr_len+lock_para.server3_addr_len, 2);
		
		memcopy(Store_LockPara+27, (unsigned char *)lock_para.server1_addr, lock_para.server1_addr_len);
		memcopy(Store_LockPara+27+lock_para.server1_addr_len, (unsigned char *)lock_para.server2_addr, lock_para.server2_addr_len);
		memcopy(Store_LockPara+27+lock_para.server1_addr_len+lock_para.server2_addr_len, (unsigned char *)lock_para.server3_addr, lock_para.server3_addr_len);
		
		len=27+lock_para.server1_addr_len+lock_para.server2_addr_len+lock_para.server3_addr_len;
		//д��
		ee_WriteBytes(Store_LockPara, LOCK_PARA_ADDR, len);
		//����
		ee_ReadBytes(temp_buf, LOCK_PARA_ADDR, len);
		
		if(Buffercmp(Store_LockPara,temp_buf,len))
		{Result=RES_SUCCESS;}
		else
		{Result=RES_COMP_FAIL;}
	}
}
//��ѯ������
void Serial3_QUERY_LOCK(void)
{
	if(ERROR == Check_SN())
		Result=RES_SN_ERROR;
	else
		Result=RES_SUCCESS;
}
//Ӧ�����������úͲ�ѯ
void ACK_Serial3_LOCK_PARA(uint8_t command,uint8_t state)
{
	u8 para_len,temp_crc;
	u8 temp_sn[6],temp_buf[100],temp_tmpbuf[150];
	uint16_t length;
	uint16_t hexpt=0,temp_hexpt=0;

	hexpt=0;
	temp_tmpbuf[hexpt] = 0xA5;	hexpt++;														//hexpt=1
	temp_tmpbuf[hexpt] = 0xA5;	hexpt++;														//hexpt=2
  hexpt+=2; temp_hexpt = 	hexpt;																	//hexpt=4
	temp_tmpbuf[hexpt] = command;	hexpt++;													//hexpt=5
	
	ee_ReadBytes(temp_sn, LOCKSN_ADDR1, 6);
	memcopy(temp_tmpbuf+hexpt,temp_sn,sizeof_lock_sn);
	hexpt+=sizeof_lock_sn;																					//hexpt=11
	temp_tmpbuf[hexpt] = state;	hexpt++;                            //hexpt=12
	if(Result == RES_SUCCESS)//�ɹ�����ȡflash����Ȼ��ش�
	{
		ee_ReadBytes(temp_buf, LOCK_PARA_ADDR+18, 3);//����3��ip��ַ�ĳ���
		para_len = temp_buf[0]+temp_buf[1]+temp_buf[2]+27;
		ee_ReadBytes(temp_buf, LOCK_PARA_ADDR, para_len);//�������в���
		
		memcopy(temp_tmpbuf+hexpt, temp_buf, 4);	hexpt+=4;
		memcopy(temp_tmpbuf+hexpt, temp_buf+4, 4);	hexpt+=4;
		memcopy(temp_tmpbuf+hexpt, temp_buf+8, 4);	hexpt+=4;
		memcopy(temp_tmpbuf+hexpt, temp_buf+12, 4);	hexpt+=4;
		memcopy(temp_tmpbuf+hexpt, temp_buf+16, 1);	hexpt+=1;
		memcopy(temp_tmpbuf+hexpt, temp_buf+17, 1);	hexpt+=1;
		
		memcopy(temp_tmpbuf+hexpt, temp_buf+18, 1);	hexpt+=1;
		memcopy(temp_tmpbuf+hexpt, temp_buf+27, lock_para.server1_addr_len);hexpt+=lock_para.server1_addr_len;
		memcopy(temp_tmpbuf+hexpt, temp_buf+21, 2);	hexpt+=2;
		
		memcopy(temp_tmpbuf+hexpt, temp_buf+19, 1);	hexpt+=1;
		memcopy(temp_tmpbuf+hexpt, temp_buf+27+lock_para.server1_addr_len, lock_para.server2_addr_len);hexpt+=lock_para.server2_addr_len;
		memcopy(temp_tmpbuf+hexpt, temp_buf+23, 2);	hexpt+=2;
		
		memcopy(temp_tmpbuf+hexpt, temp_buf+20, 1);	hexpt+=1;
		memcopy(temp_tmpbuf+hexpt, temp_buf+27+lock_para.server1_addr_len+lock_para.server2_addr_len, lock_para.server3_addr_len);hexpt+=lock_para.server3_addr_len;
		memcopy(temp_tmpbuf+hexpt, temp_buf+25, 2);	hexpt+=2;		
	}
	else	//ʧ�ܣ�����ȫΪ0
	{
		memset(temp_tmpbuf+hexpt,0x00,30);hexpt+=30;
	}
	
	memset(temp_tmpbuf+hexpt,0xff,sizeof_reserve);
	hexpt+=sizeof_reserve;																					//hexpt=22
	//���㳤��
	length = hexpt;
	//�������֡����
 	temp_tmpbuf[temp_hexpt-2] = length / 256;
 	temp_tmpbuf[temp_hexpt-1] = length & 0xFF;
	//����CRC
	temp_crc = Do_XOR(temp_tmpbuf,length);
	temp_tmpbuf[hexpt] = temp_crc;	hexpt++;          							//hexpt=23
	//����Ӧ������֡
	Usart_SendArray(USART3, temp_tmpbuf, hexpt);	
}
//����3Ӧ����
void Serial3_TxED(void)
{
	switch(Ack_State)
	{
		case WRITE_SN_OK:
		{
			ACK_Serial3_Write_SN(CMD_WRITE_SN_ACK,Result);
			break;
		}
		case SET_LOCK_OK:
		{
			ACK_Serial3_LOCK_PARA(CMD_SET_QUERY_ACK,Result);
			break;
		}
		case QUERY_LOCK_OK:
		{
			ACK_Serial3_LOCK_PARA(CMD_SET_QUERY_ACK,Result);
			break;
		}
		default:  break;
	}
}

