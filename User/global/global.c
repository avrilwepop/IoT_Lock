#include "global.h"
#include "TTLM.H"
#include "gbprotocol.h"
#include "rfid.h"
#include "gps.h"
#include "config.h"
#include "crcencoder.h"
#include "gprs.h"
#include "BSprotocol.h"
#include "common.h"
#include "lp_mode.h"
#include "bsp_ctr.h"
#include "serial.h"
#include "lcd.h"

u16 Rx1Num;
u16 Rx2Num;
u16 Rx3Num;
u8 Rx1Buf[RX1_BUF_SIZE];
u8 Rx2Buf[RX2_BUF_SIZE];
u8 Rx3Buf[50];
u8 Store_LockPara[100];//27+25*3=102�ֽڣ�����������Ϣ
u8 Store_LockRecord[20];//���Ĳ�����¼
u8 Store_GPSPara[28];//��λ����

u8 LCD_BL_timeout;
u8 KEY_reset_timeout;//�������淵�س�ʱ
u8 LockKeyOPStatus;//����������״̬
u8 keyopresult=10;//���̿����������
char LCD_buf[20];
char LCD_timebuf[32];
char LCD_keybuf[10];
char LCD_secret[10];
char LCD_blankbuf[10]={0x20,0x20,0x20,0x20,0x20};
u8 key_index=0;
u8 lcd_disp_flag=0;
u8 longpresstimeout=0;
u8 isitCkey=0;
u8 firsttimedispflag=0;
u8 delay1stick=0;

//NMEA_MSG GPS_MSG;
extern uint8_t Ack_State;

/*usart1 send start and end flags & send dwell time counter */
u8 volatile  rcv_usart1_str_flag   = 0;		//�������ݽ����������
u8 volatile  rcv_usart1_end_flag   = 0;		//�������ݽ��ս������
u8 volatile  rcv_usart1_over_count = 0;		//���ݽ���ʱ��������

/*usart2 send start and end flags & send dwell time counter */
u8 volatile  rcv_usart2_str_flag   = 0;		//�������ݽ����������
u8 volatile  rcv_usart2_end_flag   = 0;		//�������ݽ��ս������
u8 volatile  rcv_usart2_over_count = 0;		//���ݽ���ʱ��������

/*usart3 send start and end flags & send dwell time counter */
u8 volatile  rcv_usart3_str_flag   = 0;		//�������ݽ����������
u8 volatile  rcv_usart3_end_flag   = 0;		//�������ݽ��ս������
u8 volatile  rcv_usart3_over_count = 0;		//���ݽ���ʱ��������

//Һ����ʾ���Ľṹ���Ա
CN_To_Display Disp_CN[] =
{
	{  0,"�밴������������", 0, 8},
	{  1,"�������ء�������", 8, 8},
	{  2,"   ����������   ",16, 8},
	{  3,"���棡���˶Ͽ���",24, 8},
	{  4,"���棡�Ƿ���ǣ�",32, 8},
	{  5,"�����ͣ����磡",40, 8},
	{  6,"���棬�Ƿ�������",48, 8},//ȥ��
	{  7,"    �����ɹ���  ",56, 8},
	{  8,"    �������  ",64, 8},
	{  9,"    �ظ�������  ",72, 8},
	{ 10,"��������ֹ������",80, 8},
	{ 11,"  �����ɹ���    ",88, 8},
	{ 12,"ϵͳ��ʼ��������",96, 8},
	{ 13,"���Ժ�          ",104, 8},
};

//�����жӣ������������
void Do_Key(void)
{
	char key_value;
	key_value = Keyboard_Scan();
	if(isitCkey==1)//����ʱ�䵽
	{
		isitCkey=0;
		LockKeyOPStatus=sResult;
		CenterCmdOP((u8*)0,0,0,DEVICE_LOCK,KEY_OPR);// ��������
		KEY_reset_timeout=12;//�����Ϣ��3�����ʧ
	}
	if(key_value==0)	return;
	if(key_state==KEY_PRESS)
	{
		key_state=KEY_RELEASE;		//ȡֵ֮���ͷŰ���
		if((lcd_disp_flag==0)&&(KEY_LP_flag==0))
			lcd_disp_flag=1;
		KEY_LP_flag=48;								//���κΰ��������򲻽������ߡ�12s֮��û�а������£��ͽ�������
		LCD_BL_timeout=20;						//LCD������ʱ20*250=5s
		if((LockKeyOPStatus==sResult)||(LockKeyOPStatus==sAlarm))	//������棬�������棬��������
		{
			LockKeyOPStatus=sNULL;
			KEY_reset_timeout=0;
			return;
		}
		if(LockKeyOPStatus==sInput)
		{
			KEY_reset_timeout=40;	//10sû�а������£��ָ�ʱ�����
		}
		switch (key_value)
		{
			case 'C':
				if(LockKeyOPStatus==sInput)	//����״̬�£��˸�
				{
					if(key_index==0)
					{
						LockKeyOPStatus=sNULL;
						KEY_reset_timeout=0;
						return;
					}
					LCD_keybuf[key_index--]=' ';
					LCD_keybuf[key_index]='_';
				}
				else if((LockKeyOPStatus == sNULL)||(LockKeyOPStatus == sPress))	//�ڽ��������������û�����ϵ�״̬�£�����C����ʾ�����ؼ�������
				{
					if((LockStatus==SEAL)||(LockStatus==EXALARM))
						return;
					else if((LockStatus==ALARM)||vddlow_flag)	//20180326,������ ��ʾ���屨������.�͵�ѹ�� ��ʾ�͵�ѹ��ʾ
					{
						LockKeyOPStatus=sAlarm;
						KEY_reset_timeout=12;//������Ϣ��3�����ʧ
						return;
					}
					else
					{
						KEY_reset_timeout=40;
						LockKeyOPStatus = sPress;
						isitCkey=8;
					}
				}
				break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				isitCkey=0;
				if((LockStatus==ALARM)||vddlow_flag)//����״̬�£����������ּ�����ʾ��������
				{
					LockKeyOPStatus=sAlarm;
					KEY_reset_timeout=12;//������Ϣ��3�����ʧ
					return;
				}
				if((key_index<5)&&(LockKeyOPStatus==sInput))
				{
					LCD_keybuf[key_index] ='*';
					LCD_secret[key_index++]=key_value;
					LCD_keybuf[key_index] = '_';
					return;
				}
				else if((key_index==5)&&(LockKeyOPStatus==sInput))
				{
					LCD_keybuf[key_index]='*';
					LCD_secret[key_index++]=key_value;
					return;
				}
				else if(LockKeyOPStatus!=sInput)
				{
					LockKeyOPStatus=sNULL;
					return;		//��������״̬�£������ּ���Ч
				}
				break;
			case 'O':
				isitCkey=0;
				if(LockKeyOPStatus==sPress)
				{
					LockKeyOPStatus=sNULL;
					return;
				}
				if((LockKeyOPStatus==sInput)&&(key_index==6))
				{
					key_index=0;
//					printf("\r\nQ:%d%d%d%d%d%d\r\n",LCD_secret[0],LCD_secret[1],LCD_secret[2],LCD_secret[3],LCD_secret[4],LCD_secret[5]);
					CenterCmdOP((u8*)LCD_secret,0,0,DEVICE_UNLOCK,KEY_OPR);
					LockKeyOPStatus=sResult;
					KEY_reset_timeout=12;//�����Ϣ��3�����ʧ
					return;
				}
				else if((LockKeyOPStatus==sInput)&&(key_index<6))
				{return;}

				if(LockStatus==ALARM)//��������
				{
					//LockKeyOPStatus=sNULL;
					LockKeyOPStatus=sAlarm;
					KEY_reset_timeout=12;//��ʾ�б���������������3�����ʧ
					return;
				}
				else			//������,��������
				{
					KEY_reset_timeout=40;
					LockKeyOPStatus=sInput;
					sprintf(LCD_keybuf,"_     ");
					return;
				}
			default:
				isitCkey=0;
				break;
		}
	}
}

//�����жӣ�LCD��ʾ�������
void DO_Display(void)
{
	u8 i,tempbuf[6];

	if(KEY_LP_flag==0)
	{
		LCD_Disp_Off();
		return;
	}
	
	if(delay1stick<12)
	{
		LCD_Show_ASCII(0, 0, "                ",font_8x16,1);
		LCD_Show_ASCII(3, 0, "                ",font_8x16,1);
		LCD_ShowCN(1, 0, Disp_CN[12].start_pos, Disp_CN[12].lenth, 1);
		LCD_ShowCN(2, 0, Disp_CN[13].start_pos, Disp_CN[13].lenth, 1);
		LCD_Refresh_Gram();
		return;
	}
	else if((delay1stick>=12)&&(delay1stick<30))
	{
		delay1stick=200;
	}

	if(firsttimedispflag==0)
	{
		firsttimedispflag=1;
		LCD_Fill_All_nul(0x00);  
	}
	
	if(lcd_disp_flag==1)//�������Ѻ󣬴�LCD��ʾ
	{
		lcd_disp_flag=0;
		LCD_Disp_On();
	}

	if(KEY_reset_timeout==0)
	{
		LockKeyOPStatus=sNULL;
		key_index=0;
		keyopresult=0;
		sprintf(LCD_keybuf,"_     ");
	}

	if(CSQValue <=10)
		LCD_ShowIcon(0, 0, 1, 3); //1
	else if(CSQValue <=20)
		LCD_ShowIcon(0, 0, 1, 2);//2
	else if(CSQValue <=31)
		LCD_ShowIcon(0, 0, 1, 1);//3
	else
		LCD_ShowIcon(0, 0, 1, 4); //x
	if((CSQValue ==99)&&(TCPIP_been_OK ==3 ))
	LCD_ShowIcon(0, 0, 1, 1);//3

	if(TCPIP_been_OK ==3 )//ͨѶͼ��
		LCD_ShowIcon(0, 1, 1, 5);
	else
		LCD_ShowIcon(0, 1, 1, 0);

	if((GPS_Valid_flag)||(GPS_disp_flag))//��λͼ��
		LCD_ShowIcon(0, 2, 1, 6);
	else
		LCD_ShowIcon(0, 2, 1, 0);

	if(LockStatus == SEAL)//������ͼ��
		LCD_ShowIcon(0, 3, 1, 7);
	else if(LockStatus == UNSEAL)
		LCD_ShowIcon(0, 3, 1, 8);

	if(LockStatus == ALARM)//������ͼ��
	{
		LCD_ShowIcon(0, 3, 1, 7);//����
		LCD_ShowIcon(0, 4, 1, 9);
	}
	else
		LCD_ShowIcon(0, 4, 1, 0);

	if(Adc_Vdd >= 4000)//��ص�ͼ��
		LCD_ShowIcon(0, 7, 1, 10);
	else if(Adc_Vdd >= 3800)
		LCD_ShowIcon(0, 7, 1, 11);
	else if(Adc_Vdd >= 3550)
		LCD_ShowIcon(0, 7, 1, 12);
	else
		LCD_ShowIcon(0, 7, 1, 13);

	LCD_Show_ASCII(2, 0, "    SN:19000000001",font_6x12,1);

	if(LockKeyOPStatus==sNULL)
	{
		RTC_OP((u8*)tempbuf,RTC_GETTIME_BCD);
		for(i=0;i<6;i++)
			tempbuf[i] = BcdToHex(tempbuf[i]);
		sprintf(LCD_timebuf,"   20%02d-%02d-%02d       %02d:%02d:%02d    ",tempbuf[0],tempbuf[1],tempbuf[2],tempbuf[3],tempbuf[4],tempbuf[5]);
		LCD_Show_ASCII(2, 0, LCD_timebuf,font_8x16,1);
	}
	else if(LockKeyOPStatus==sInput)
	{
		LCD_ShowCN(2, 0, Disp_CN[2].start_pos, Disp_CN[2].lenth, 1);//��������
		LCD_Show_ASCII(3, 0, LCD_blankbuf,font_8x16,1);
		LCD_Show_ASCII(3, 5, (char *)LCD_keybuf,font_8x16,1);
		LCD_Show_ASCII(3, 11, LCD_blankbuf,font_8x16,1);
	}
	else if(LockKeyOPStatus==sResult)
	{
		if(keyopresult==0)//�����ɹ�
			LCD_ShowCN(2, 0, Disp_CN[7].start_pos, Disp_CN[7].lenth, 1);
		else if(keyopresult==1)//�������
			LCD_ShowCN(2, 0, Disp_CN[8].start_pos, Disp_CN[8].lenth, 1);
		else if(keyopresult==2)//�ظ����
			LCD_ShowCN(2, 0, Disp_CN[9].start_pos, Disp_CN[9].lenth, 1);
		else if(keyopresult==3)//�б���  ��ֹ����
			LCD_ShowCN(2, 0, Disp_CN[10].start_pos, Disp_CN[10].lenth, 1);
		else if(keyopresult==4)//�����ɹ���
			LCD_ShowCN(2, 0, Disp_CN[11].start_pos, Disp_CN[11].lenth, 1);
		LCD_Fill_Area(0, 48, 128, 64, 0);
	}
	else if(LockKeyOPStatus==sPress)
	{
		LCD_ShowCN(2, 0, Disp_CN[1].start_pos, Disp_CN[1].lenth, 1);
		LCD_Fill_Area(0, 48, 128, 64, 0);
	}
	else if(LockKeyOPStatus==sAlarm)
	{
		if(AlarmType&OPENALARM)//�Ƿ�����-����
			LCD_ShowCN(2, 0, Disp_CN[3].start_pos, Disp_CN[3].lenth, 1);
		else if(AlarmType&CKALARM)//�Ƿ����-����
			LCD_ShowCN(2, 0, Disp_CN[4].start_pos, Disp_CN[4].lenth, 1);
		else if(vddlow_flag)//�͵�ѹ��ʾ
			LCD_ShowCN(2, 0, Disp_CN[5].start_pos, Disp_CN[5].lenth, 1);
		LCD_Fill_Area(0, 48, 128, 64, 0);
	}
	
	if(LCD_BL_timeout)
		LCD_BL_ON;
	else
		LCD_BL_OFF;

	LCD_Refresh_Gram();
}

//�����жӣ�����1������򣬴���GPRS����
void DO_USART1_GPRS(void)
{
	if((NetModuleSTATUS!=M35_KEEPOFF)&&(NetModuleSTATUS!=LOWVDD_PWR_OFF))//M26�Ƿ�����
	{
		if(rcv_usart1_end_flag)				  			//���ݽ������
		{
			GPRS_DTR_L;
			GPRS_LP_flag=0;
			Lowpowertick = 0;
			Check_GPRS_Data(Rx1Buf,Rx1Num);//����GPRS����Ϣ���ս������
			rcv_usart1_end_flag = 0;
		}
		AT_TASK();//����ATָ��
	}
}
//�����жӣ�����2������򣬴���GPS����
void DO_USART2_GPS(void)
{
	if(rcv_usart2_end_flag)
	{
		Gps_Handler();
		//UARTWrite((u8*)Rx2Buf,Rx2Num,0);
		rcv_usart2_end_flag = 0;
		Rx2Num=0;
	}
}
//�����жӣ�����3������򣬽�����λ��������
void DO_USART3_PC(void)
{
	if(rcv_usart3_end_flag)
	{
//		Ack_State = Serial3_RxCB();
//		Serial3_TxED();
		//AT_DirectOP(Rx3Buf,Rx3Num);	//͸����GPRSģ��
		if(Rx3Buf[0]=='$')
		{
			Usart_SendString(USART2, (char *)Rx3Buf);//͸����GPSģ�飬��������GPS
		}
		else if(Rx3Buf[0]=='Z')
		{
			DeviceRegisteOperate();//GPRSע��
		}
		else
		{
			GPRS_DTR_L;
			GPRS_LP_flag=0;
			Lowpowertick = 0;
			Usart_SendString(USART1, (char *)Rx3Buf);
		}

		rcv_usart3_end_flag = 0;
		Rx3Num=0;
	}
}
//�����жӣ�MCU���ߴ�������ж��Ƿ����ߣ��Լ����Ѻ����������
void DO_LOWPOWER_MODE(void)
{
	//�������������Ҫ��Ӹ��ֱ�־λ�Ĵ����ж��Ƿ���Խ���͹���ģʽ
	//������Խ���͹��ģ���CheckLowPower��λ

	CheckForSleep();

	if(CheckLowPower)
	{
		CheckLowPower=0;
		LPMODE_OP();
	}
}

