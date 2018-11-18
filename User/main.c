#include "main.h"
#include "TTLM.H"
#include "gprs.H"
#include "BSprotocol.h"
#include "global.h"
#include "gps.h"

//�������нṹ���Ա 
static TASK_COMPONENTS TaskComps[] = 
{
	{0, 20, 20,Do_Key},
	{0,200,200,DO_Display},
	{0, 10, 10,DO_USART2_GPS},
	{0, 10, 10,DO_USART3_PC},
	{0, 50, 50,DO_LOWPOWER_MODE},
};

//ʱ���־����
void TaskReMarks(void)
{
	unsigned char i = 0;
	
	for(i = 0; i< TASKS_MAX; i++)
	{
		if(TaskComps[i].Timer)
		{
			TaskComps[i].Timer--;
			if(TaskComps[i].Timer == 0)
			{
				TaskComps[i].Timer = TaskComps[i].ItvTimer;
				TaskComps[i].Run = 1;   //�����������
			}
		}
	}
}

//�����־����
void TaskProcess(void)
{
	unsigned char i = 0;

	for(i = 0;i < TASKS_MAX;i++)
	{
		if(TaskComps[i].Run)
		{
			TaskComps[i].TaskHook();
			TaskComps[i].Run = 0;
		}
	}
}

void Peripheral_Init(void)
{ 
	BSP_CTR_Config();
	ADC1_Init();
	Timer2_Config();
	USART1_Config(57600);
	USART2_Config(9600);
	USART3_Config(115200);
	RTC_Config();
	LCD_GPIO_Config();
	LCD_Show_Logo();
	LCD_Ini();				//��ʼ��֮����ʾlogo
	LCD_BL_timeout=30;
	KEY_LP_flag=48;
	I2C_GPIO_Config();
	NRF905_IOinit();
	RFM_Ini(LockCHcfg);
#if DEBUG_MODE
	WTDG_DISABLE;
#else
	WTDG_ENABLE;
#endif
}

int main(void)
{
	Peripheral_Init();
#if 0
	Lock_EProm_Reset(0);
#endif
	Lock_Para_Init();
	E2PROM_check();

#if NEED_GPS
	GPS_SW(1);//gps����
#endif

#if NEED_GPRS
	NetModulePowerOP(HD_PWR_ON);//gprs����
#endif

	POWER_CTR_ON;

	while (1)
	{
		TaskProcess();		//������ѯ����
		RF_Process();			//RF���ݴ���������
#if NEED_GPRS
		LockTimHandler();	//GPRS�е�ָ����Ҫ�ط�
		DO_USART1_GPRS();	//GPRS��������������
#endif
		PsecondEventHandler();

		FEED_WTDG;
	}
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
