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
u8 Store_LockPara[100];//27+25*3=102字节，锁的设置信息
u8 Store_LockRecord[20];//锁的操作记录
u8 Store_GPSPara[28];//定位数据

u8 LCD_BL_timeout;
u8 KEY_reset_timeout;//按键界面返回超时
u8 LockKeyOPStatus;//按键操作的状态
u8 keyopresult=10;//键盘开锁操作结果
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
u8 volatile  rcv_usart1_str_flag   = 0;		//串口数据接收启动标记
u8 volatile  rcv_usart1_end_flag   = 0;		//串口数据接收结束标记
u8 volatile  rcv_usart1_over_count = 0;		//数据接收时间溢出标记

/*usart2 send start and end flags & send dwell time counter */
u8 volatile  rcv_usart2_str_flag   = 0;		//串口数据接收启动标记
u8 volatile  rcv_usart2_end_flag   = 0;		//串口数据接收结束标记
u8 volatile  rcv_usart2_over_count = 0;		//数据接收时间溢出标记

/*usart3 send start and end flags & send dwell time counter */
u8 volatile  rcv_usart3_str_flag   = 0;		//串口数据接收启动标记
u8 volatile  rcv_usart3_end_flag   = 0;		//串口数据接收结束标记
u8 volatile  rcv_usart3_over_count = 0;		//数据接收时间溢出标记

//液晶显示中文结构体成员
CN_To_Display Disp_CN[] =
{
	{  0,"请按“开”键开锁", 0, 8},
	{  1,"长按“关”键关锁", 8, 8},
	{  2,"   请输入密码   ",16, 8},
	{  3,"警告！锁杆断开！",24, 8},
	{  4,"警告！非法拆壳！",32, 8},
	{  5,"电量低，请充电！",40, 8},
	{  6,"警告，非法开锁！",48, 8},//去掉
	{  7,"    开锁成功！  ",56, 8},
	{  8,"    密码错误！  ",64, 8},
	{  9,"    重复开锁！  ",72, 8},
	{ 10,"报警！禁止开锁！",80, 8},
	{ 11,"  关锁成功！    ",88, 8},
	{ 12,"系统初始化。。。",96, 8},
	{ 13,"请稍候          ",104, 8},
};

//任务列队：按键处理程序
void Do_Key(void)
{
	char key_value;
	key_value = Keyboard_Scan();
	if(isitCkey==1)//长按时间到
	{
		isitCkey=0;
		LockKeyOPStatus=sResult;
		CenterCmdOP((u8*)0,0,0,DEVICE_LOCK,KEY_OPR);// 按键关锁
		KEY_reset_timeout=12;//结果信息，3秒后消失
	}
	if(key_value==0)	return;
	if(key_state==KEY_PRESS)
	{
		key_state=KEY_RELEASE;		//取值之后释放按键
		if((lcd_disp_flag==0)&&(KEY_LP_flag==0))
			lcd_disp_flag=1;
		KEY_LP_flag=48;								//有任何按键按下则不进入休眠。12s之内没有按键按下，就进入休眠
		LCD_BL_timeout=20;						//LCD背光延时20*250=5s
		if((LockKeyOPStatus==sResult)||(LockKeyOPStatus==sAlarm))	//结果界面，报警界面，按键返回
		{
			LockKeyOPStatus=sNULL;
			KEY_reset_timeout=0;
			return;
		}
		if(LockKeyOPStatus==sInput)
		{
			KEY_reset_timeout=40;	//10s没有按键按下，恢复时间界面
		}
		switch (key_value)
		{
			case 'C':
				if(LockKeyOPStatus==sInput)	//输入状态下，退格
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
				else if((LockKeyOPStatus == sNULL)||(LockKeyOPStatus == sPress))	//在界面待机，而且锁没有锁上的状态下，按下C，显示长按关键关锁。
				{
					if((LockStatus==SEAL)||(LockStatus==EXALARM))
						return;
					else if((LockStatus==ALARM)||vddlow_flag)	//20180326,报警下 显示具体报警类型.低电压下 显示低电压提示
					{
						LockKeyOPStatus=sAlarm;
						KEY_reset_timeout=12;//报警信息，3秒后消失
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
				if((LockStatus==ALARM)||vddlow_flag)//报警状态下，按任意数字键，显示报警内容
				{
					LockKeyOPStatus=sAlarm;
					KEY_reset_timeout=12;//报警信息，3秒后消失
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
					return;		//不在输入状态下，按数字键无效
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
					KEY_reset_timeout=12;//结果信息，3秒后消失
					return;
				}
				else if((LockKeyOPStatus==sInput)&&(key_index<6))
				{return;}

				if(LockStatus==ALARM)//不允许开锁
				{
					//LockKeyOPStatus=sNULL;
					LockKeyOPStatus=sAlarm;
					KEY_reset_timeout=12;//显示有报警，不允许开锁，3秒后消失
					return;
				}
				else			//允许开锁,输入密码
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

//任务列队：LCD显示处理程序
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
	
	if(lcd_disp_flag==1)//按键唤醒后，打开LCD显示
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

	if(TCPIP_been_OK ==3 )//通讯图标
		LCD_ShowIcon(0, 1, 1, 5);
	else
		LCD_ShowIcon(0, 1, 1, 0);

	if((GPS_Valid_flag)||(GPS_disp_flag))//定位图标
		LCD_ShowIcon(0, 2, 1, 6);
	else
		LCD_ShowIcon(0, 2, 1, 0);

	if(LockStatus == SEAL)//上锁的图标
		LCD_ShowIcon(0, 3, 1, 7);
	else if(LockStatus == UNSEAL)
		LCD_ShowIcon(0, 3, 1, 8);

	if(LockStatus == ALARM)//报警的图标
	{
		LCD_ShowIcon(0, 3, 1, 7);//有锁
		LCD_ShowIcon(0, 4, 1, 9);
	}
	else
		LCD_ShowIcon(0, 4, 1, 0);

	if(Adc_Vdd >= 4000)//电池的图标
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
		LCD_ShowCN(2, 0, Disp_CN[2].start_pos, Disp_CN[2].lenth, 1);//输入密码
		LCD_Show_ASCII(3, 0, LCD_blankbuf,font_8x16,1);
		LCD_Show_ASCII(3, 5, (char *)LCD_keybuf,font_8x16,1);
		LCD_Show_ASCII(3, 11, LCD_blankbuf,font_8x16,1);
	}
	else if(LockKeyOPStatus==sResult)
	{
		if(keyopresult==0)//开锁成功
			LCD_ShowCN(2, 0, Disp_CN[7].start_pos, Disp_CN[7].lenth, 1);
		else if(keyopresult==1)//密码错误
			LCD_ShowCN(2, 0, Disp_CN[8].start_pos, Disp_CN[8].lenth, 1);
		else if(keyopresult==2)//重复解封
			LCD_ShowCN(2, 0, Disp_CN[9].start_pos, Disp_CN[9].lenth, 1);
		else if(keyopresult==3)//有报警  禁止开锁
			LCD_ShowCN(2, 0, Disp_CN[10].start_pos, Disp_CN[10].lenth, 1);
		else if(keyopresult==4)//关锁成功！
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
		if(AlarmType&OPENALARM)//非法开锁-报警
			LCD_ShowCN(2, 0, Disp_CN[3].start_pos, Disp_CN[3].lenth, 1);
		else if(AlarmType&CKALARM)//非法拆壳-报警
			LCD_ShowCN(2, 0, Disp_CN[4].start_pos, Disp_CN[4].lenth, 1);
		else if(vddlow_flag)//低电压提示
			LCD_ShowCN(2, 0, Disp_CN[5].start_pos, Disp_CN[5].lenth, 1);
		LCD_Fill_Area(0, 48, 128, 64, 0);
	}
	
	if(LCD_BL_timeout)
		LCD_BL_ON;
	else
		LCD_BL_OFF;

	LCD_Refresh_Gram();
}

//任务列队：串口1处理程序，处理GPRS数据
void DO_USART1_GPRS(void)
{
	if((NetModuleSTATUS!=M35_KEEPOFF)&&(NetModuleSTATUS!=LOWVDD_PWR_OFF))//M26是否正常
	{
		if(rcv_usart1_end_flag)				  			//数据接收完毕
		{
			GPRS_DTR_L;
			GPRS_LP_flag=0;
			Lowpowertick = 0;
			Check_GPRS_Data(Rx1Buf,Rx1Num);//所有GPRS的消息接收解包函数
			rcv_usart1_end_flag = 0;
		}
		AT_TASK();//处理AT指令
	}
}
//任务列队：串口2处理程序，处理GPS数据
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
//任务列队：串口3处理程序，接收上位机的数据
void DO_USART3_PC(void)
{
	if(rcv_usart3_end_flag)
	{
//		Ack_State = Serial3_RxCB();
//		Serial3_TxED();
		//AT_DirectOP(Rx3Buf,Rx3Num);	//透传到GPRS模块
		if(Rx3Buf[0]=='$')
		{
			Usart_SendString(USART2, (char *)Rx3Buf);//透传到GPS模块，用于设置GPS
		}
		else if(Rx3Buf[0]=='Z')
		{
			DeviceRegisteOperate();//GPRS注册
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
//任务列队：MCU休眠处理程序，判断是否休眠，以及唤醒后的重新配置
void DO_LOWPOWER_MODE(void)
{
	//这个函数里面需要添加各种标志位的处理，判断是否可以进入低功耗模式
	//如果可以进入低功耗，将CheckLowPower置位

	CheckForSleep();

	if(CheckLowPower)
	{
		CheckLowPower=0;
		LPMODE_OP();
	}
}

