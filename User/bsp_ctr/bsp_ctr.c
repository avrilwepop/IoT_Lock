#include "bsp_ctr.h"

/*******************************************************************************
* Function Name  : BSP_CTR_Config
* Description    : BSP_CTR_Config
* Input          :  
* Return         :  
*******************************************************************************/
void BSP_CTR_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA	| RCC_AHBPeriph_GPIOB	| RCC_AHBPeriph_GPIOC	| RCC_AHBPeriph_GPIOD,  ENABLE);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_USART2 | RCC_APB1Periph_USART3 , ENABLE );
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	//GPRS��������
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 ;	
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);

	//GPS��������
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Pin = GPS_EN_GPIO_PIN ;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_Init(GPS_EN_GPIO_PORT, &GPIO_InitStruct);
	GPS_POWER_OFF;

	//WTDG��������
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Pin = WTDG_EN_PIN | WTDG_IO_PIN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_Init(WTDG_EN_PORT, &GPIO_InitStruct);

	//�����������
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_40MHz;
  GPIO_Init(GPIOB,&GPIO_InitStruct);

	//�����������������
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA,&GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_14|GPIO_Pin_15;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOC,&GPIO_InitStruct);

  //POW_CUT
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_40MHz;
  GPIO_Init(GPIOA,&GPIO_InitStruct);
}

void ADC1_Init(void)//PB1����ص�ѹ
{
	GPIO_InitTypeDef  				GPIO_InitStructure;
	ADC_InitTypeDef     			ADC_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &GPIO_InitStructure);	
	
	/* ADC1 Configuration ------------------------------------------------------*/
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

  /* ADC1 DeInit */  
  ADC_DeInit(ADC1);
	
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;//12λ����
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; //�涨ģʽװ������������ģʽ
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; 
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//���ݶ���Ϊ�Ҷ���
  ADC_InitStructure.ADC_NbrOfConversion = 1;
  ADC_Init(ADC1, &ADC_InitStructure); 
	
  ADC_RegularChannelConfig(ADC1, ADC_Channel_9 , 1,ADC_SampleTime_96Cycles); /* Convert the ADC1 Channel 11 with 239.5 Cycles as sampling time */  
	
	 /* Define delay between ADC1 conversions */
  ADC_DelaySelectionConfig(ADC1, ADC_DelayLength_Freeze);
  
  /* Enable ADC1 Power Down during Delay */
  ADC_PowerDownCmd(ADC1, ADC_PowerDown_Idle_Delay, ENABLE);
	
  ADC_Cmd(ADC1, ENABLE);  /* Enable ADCperipheral[PerIdx] */	  
  while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_ADONS)); /* Wait the ADCEN falg */
	
	/* Start ADC1 Software Conversion */
  //ADC_SoftwareStartConv(ADC1);
}

/*******************************************************************************
* Function Name  : Get_Adc
* Description    : ��ȡADת�����
* Input          : ch��ͨ��.
* Return         : ת�����.
*******************************************************************************/
u16 Get_Adc(void)   
{  	
	ADC_SoftwareStartConv(ADC1);		//ʹ��ָ����ADC1�����ת����������	
	FEED_WTDG;
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//�ȴ�ת������

	return ADC_GetConversionValue(ADC1);	//�������һ��ADC1�������ת�����
}

/*******************************************************************************
* Function Name  : InsertSort
* Description    : ֱ�Ӳ��뷨����
* Input          : a[]���������飻count�����鳤��.
* Return         : NONE.
*******************************************************************************/
void InsertSort(u16 a[],u8 count)  
{  
	signed char i,j;
	u16 temp; 
	for(i=1;i<count;i++)     
	{  
	   temp=a[i];  
	   j=i-1;  
	   while((a[j]>temp)&&(j>=0))  
	   {  
	     a[j+1]=a[j];  
	      j--;  
	   }  
	   if(j!=(i-1))       
	     a[j+1]=temp;  
	 }
}

/*******************************************************************************
* Function Name  : DO_BatVol
* Description    : ��ȡ��ѹֵ
* Input          : NONE.
* Return         : ��ѹֵ.
*******************************************************************************/
u16 DO_BatVol1(void)
{
	u16 result[10]={0};
	u32 sum=0,average=0;
	u8 i;

	for(i=0;i<10;i++)
	{
		result[i]=Get_Adc();
		SysTick_Delay_Ms(1);
	}
	InsertSort(result,10); /*����*/
	for(i=1;i<9;i++)		/*ȥ�������Сֵ���*/
	{
		sum+=result[i];
	}
	average=sum/8;	/*ȡƽ��ֵ*/

	result[0]=average * 6600 / 4096;
	return (result[0]);
}

u16 DO_BatVol(void)
{
	u16 result;

	result=Get_Adc();
	result=result * 6600 / 4096;
	return result;
}


