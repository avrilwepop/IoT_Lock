#include "key.h"

eKey_sta key_state;

//������ɨ��ʱ�������������룬��������͵�ƽ
static void RowScan_GPIO_Config(void)   
{
	GPIOC->MODER |= 0x55;
	GPIOC->MODER &= 0xFFFFC055;
	GPIOC->OSPEEDR |= 0xFF;
	GPIOC->PUPDR |= 0x1555;
	GPIOC->PUPDR &= 0xFFFFD555;
	GPIOC->ODR &= 0xFF00;

//	GPIO_InitTypeDef GPIO_InitStruct;
//	
//	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;		//3����������Ϊ��������
//	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
//	GPIO_InitStruct.GPIO_Pin = Row1_GPIO_PIN | Row2_GPIO_PIN | Row3_GPIO_PIN;			//ѡ��pin
//	GPIO_Init(GPIOC, &GPIO_InitStruct);		//��ʼ������GPIO
//	
//	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;		//4����������Ϊ���
//	GPIO_InitStruct.GPIO_Pin = Col1_GPIO_PIN | Col2_GPIO_PIN | Col3_GPIO_PIN | Col4_GPIO_PIN;			//ѡ��pin
//  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_40MHz;
//	GPIO_Init(GPIOC, &GPIO_InitStruct);		//��ʼ��GPIO
//	
//	GPIO_ResetBits(GPIOC, Col1_GPIO_PIN | Col2_GPIO_PIN | Col3_GPIO_PIN | Col4_GPIO_PIN);//��ʼ��֮�󣬽���������Ϊ�͵�ƽ
	
}

//������ɨ��ʱ�������������룬��������͵�ƽ
static void ColScan_GPIO_Config(void)
{
	GPIOC->MODER |= 0x1500;
	GPIOC->MODER &= 0xFFFFD500;
	GPIOC->OSPEEDR |= 0x3F00;
	GPIOC->PUPDR |= 0x55;
	GPIOC->PUPDR &= 0xFFFFC055;
	
//	GPIO_InitTypeDef GPIO_InitStruct;
//	
//	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;		//4����������Ϊ���
//	GPIO_InitStruct.GPIO_Pin = Row1_GPIO_PIN | Row2_GPIO_PIN | Row3_GPIO_PIN;			//ѡ��pin
//	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_40MHz;
//	GPIO_Init(GPIOC, &GPIO_InitStruct);		//��ʼ������GPIO
//	
//	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;		//4����������Ϊ��������
//	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
//	GPIO_InitStruct.GPIO_Pin = Col1_GPIO_PIN | Col2_GPIO_PIN | Col3_GPIO_PIN | Col4_GPIO_PIN;			//ѡ��pin
//	GPIO_Init(GPIOC, &GPIO_InitStruct);		//��ʼ��GPIO	
}

char Keyboard_Scan(void)
{
	uint16_t temp;
	uint8_t value=0;
	
	RowScan_GPIO_Config();
	
	temp = GPIOC->IDR;
	temp &= ( Row1_GPIO_PIN | Row2_GPIO_PIN | Row3_GPIO_PIN );
	
	switch (temp)
	{
		case (Row2_GPIO_PIN | Row3_GPIO_PIN ): value = 1; break;  //��1���а���������
		case (Row1_GPIO_PIN | Row3_GPIO_PIN ): value = 2; break;  //��2���а���������
		case (Row1_GPIO_PIN | Row2_GPIO_PIN ): value = 3; break;  //��3���а���������
		default: key_state=KEY_NULL;return 0; //û�а������£�����ɨ�躯��
	}
	ColScan_GPIO_Config();
	GPIOC->ODR = temp;//��ɨ��ʱ�����а������µ���һ������Ϊ����͵�ƽ������������Ϊ�ߵ�ƽ��
	
	temp = GPIOC->IDR;
	temp &= ( Col1_GPIO_PIN | Col2_GPIO_PIN | Col3_GPIO_PIN | Col4_GPIO_PIN );
	
	switch (temp)
	{
		case (Col2_GPIO_PIN | Col3_GPIO_PIN | Col4_GPIO_PIN): value = (value-1)*4+1; break;  //��1���а���������
		case (Col1_GPIO_PIN | Col3_GPIO_PIN | Col4_GPIO_PIN): value = (value-1)*4+2; break;  //��2���а���������
		case (Col1_GPIO_PIN | Col2_GPIO_PIN | Col4_GPIO_PIN): value = (value-1)*4+3; break;  //��3���а���������
		case (Col1_GPIO_PIN | Col2_GPIO_PIN | Col3_GPIO_PIN): value = (value-1)*4+4; break;  //��4���а���������
		default: key_state=KEY_NULL;return 0; //û�а������£�����ɨ�躯��
	}
	if(key_state==KEY_RELEASE)	return 0;//��ֵȡ��֮�󣬵��ͷ�״̬������ȡ����ֵ
	if(key_state==KEY_NULL)	key_state=KEY_TOUCH;//û�а������µ�״̬��  ����а����Ӵ���״̬������Ҫȥ����
	if(key_state<KEY_DEBOUNCE)	return 0;//����ȥ������20ms
	else
	{
		key_state=KEY_PRESS;				//ͨ��ȥ������20ms�����ж�Ϊ�а�������
		switch (value)
		{
			case 1: return 0x31;
			case 2: return 0x32;
			case 3: return 0x33;
			case 4: return 0x34;
			case 5: return 0x35;
			case 6: return 0x36;
			case 7: return 0x37;
			case 8: return 0x38;
			case 9: return 'O';
			case 10: return 0x30;
			case 11: return 0x39;
			case 12: return 'C';
			
			default: return 0; //���µĲ��������������͵���û�а���
		}
	}
}


