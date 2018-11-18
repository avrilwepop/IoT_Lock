#include "systick.h"

/*微秒级精确延时函数*/
//void SysTick_Delay_Us( __IO uint32_t us)
//{
//	uint32_t i;
//	SysTick_Config(sysclk/1000000);
//	
//	for(i=0;i<us;i++)
//	{
//		// 当计数器的值减小到0的时候，CRTL寄存器的位16会置1	
//		while( !((SysTick->CTRL)&(1<<SysTick_CTRL_COUNTFLAG_Pos)) );
//	}
//	// 关闭SysTick定时器
//	SysTick->CTRL &=~SysTick_CTRL_ENABLE_Msk;
//}

///*毫秒级精确延时函数*/
//void SysTick_Delay_Ms( __IO uint32_t ms)
//{
//	uint32_t i;	
//	SysTick_Config(sysclk/1000);
//	
//	for(i=0;i<ms;i++)
//	{
//		// 当计数器的值减小到0的时候，CRTL寄存器的位16会置1
//		// 当置1时，读取该位会清0
//		while( !((SysTick->CTRL)&(1<<SysTick_CTRL_COUNTFLAG_Pos)) );
//	}
//	// 关闭SysTick定时器
//	SysTick->CTRL &=~ SysTick_CTRL_ENABLE_Msk;
//}

//微秒级的延时
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

//毫秒级的延时
void SysTick_Delay_Ms(__IO uint32_t ms)
{
	u16 i=0;
	while(ms--)
	{
		i=3958;  //自己定义
		while(i--) ;    
	}
}

