#include "systick.h"

/*΢�뼶��ȷ��ʱ����*/
//void SysTick_Delay_Us( __IO uint32_t us)
//{
//	uint32_t i;
//	SysTick_Config(sysclk/1000000);
//	
//	for(i=0;i<us;i++)
//	{
//		// ����������ֵ��С��0��ʱ��CRTL�Ĵ�����λ16����1	
//		while( !((SysTick->CTRL)&(1<<SysTick_CTRL_COUNTFLAG_Pos)) );
//	}
//	// �ر�SysTick��ʱ��
//	SysTick->CTRL &=~SysTick_CTRL_ENABLE_Msk;
//}

///*���뼶��ȷ��ʱ����*/
//void SysTick_Delay_Ms( __IO uint32_t ms)
//{
//	uint32_t i;	
//	SysTick_Config(sysclk/1000);
//	
//	for(i=0;i<ms;i++)
//	{
//		// ����������ֵ��С��0��ʱ��CRTL�Ĵ�����λ16����1
//		// ����1ʱ����ȡ��λ����0
//		while( !((SysTick->CTRL)&(1<<SysTick_CTRL_COUNTFLAG_Pos)) );
//	}
//	// �ر�SysTick��ʱ��
//	SysTick->CTRL &=~ SysTick_CTRL_ENABLE_Msk;
//}

//΢�뼶����ʱ
void SysTick_Delay_Us(__IO uint32_t us)
{
	while(us--)
	{
		__NOP();
		__NOP();
		__NOP();
		__NOP(); 
	}
}

//���뼶����ʱ
void SysTick_Delay_Ms(__IO uint32_t ms)
{
	u16 i=0;
	while(ms--)
	{
		i=3958;  //�Լ�����
		while(i--) ;    
	}
}

