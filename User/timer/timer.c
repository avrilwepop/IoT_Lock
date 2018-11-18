#include "timer.h"

/*configur the timer2*/
void Timer2_Config(void)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	TIM_TimeBaseStructure.TIM_Period = 10-1;			    		/*计数值 1mS*/  
	TIM_TimeBaseStructure.TIM_Prescaler = 3200-1;    	    /*预分频,此值+1为分频的除数*/
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;  	
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 	
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	
	/*preload enable*/
	TIM_ARRPreloadConfig(TIM2, ENABLE);

	/*clear the pending bit*/
	TIM_ClearITPendingBit(TIM2,  TIM_IT_Update); 

	/*4 channels in the interruption mode*/
	TIM_ITConfig(TIM2,  TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM2, ENABLE);
}


