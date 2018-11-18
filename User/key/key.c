#include "key.h"

eKey_sta key_state;

//进行行扫描时，行线上拉输入，列线输出低电平
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
//	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;		//3根行线配置为上拉输入
//	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
//	GPIO_InitStruct.GPIO_Pin = Row1_GPIO_PIN | Row2_GPIO_PIN | Row3_GPIO_PIN;			//选择pin
//	GPIO_Init(GPIOC, &GPIO_InitStruct);		//初始化行线GPIO
//	
//	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;		//4根列线配置为输出
//	GPIO_InitStruct.GPIO_Pin = Col1_GPIO_PIN | Col2_GPIO_PIN | Col3_GPIO_PIN | Col4_GPIO_PIN;			//选择pin
//  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_40MHz;
//	GPIO_Init(GPIOC, &GPIO_InitStruct);		//初始化GPIO
//	
//	GPIO_ResetBits(GPIOC, Col1_GPIO_PIN | Col2_GPIO_PIN | Col3_GPIO_PIN | Col4_GPIO_PIN);//初始化之后，将列线设置为低电平
	
}

//进行列扫描时，列线上拉输入，行线输出低电平
static void ColScan_GPIO_Config(void)
{
	GPIOC->MODER |= 0x1500;
	GPIOC->MODER &= 0xFFFFD500;
	GPIOC->OSPEEDR |= 0x3F00;
	GPIOC->PUPDR |= 0x55;
	GPIOC->PUPDR &= 0xFFFFC055;
	
//	GPIO_InitTypeDef GPIO_InitStruct;
//	
//	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;		//4根行线配置为输出
//	GPIO_InitStruct.GPIO_Pin = Row1_GPIO_PIN | Row2_GPIO_PIN | Row3_GPIO_PIN;			//选择pin
//	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_40MHz;
//	GPIO_Init(GPIOC, &GPIO_InitStruct);		//初始化行线GPIO
//	
//	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;		//4根列线配置为上拉输入
//	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
//	GPIO_InitStruct.GPIO_Pin = Col1_GPIO_PIN | Col2_GPIO_PIN | Col3_GPIO_PIN | Col4_GPIO_PIN;			//选择pin
//	GPIO_Init(GPIOC, &GPIO_InitStruct);		//初始化GPIO	
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
		case (Row2_GPIO_PIN | Row3_GPIO_PIN ): value = 1; break;  //第1行有按键被按下
		case (Row1_GPIO_PIN | Row3_GPIO_PIN ): value = 2; break;  //第2行有按键被按下
		case (Row1_GPIO_PIN | Row2_GPIO_PIN ): value = 3; break;  //第3行有按键被按下
		default: key_state=KEY_NULL;return 0; //没有按键按下，结束扫描函数
	}
	ColScan_GPIO_Config();
	GPIOC->ODR = temp;//列扫描时，将有按键按下的那一行设置为输出低电平，其他行设置为高电平。
	
	temp = GPIOC->IDR;
	temp &= ( Col1_GPIO_PIN | Col2_GPIO_PIN | Col3_GPIO_PIN | Col4_GPIO_PIN );
	
	switch (temp)
	{
		case (Col2_GPIO_PIN | Col3_GPIO_PIN | Col4_GPIO_PIN): value = (value-1)*4+1; break;  //第1列有按键被按下
		case (Col1_GPIO_PIN | Col3_GPIO_PIN | Col4_GPIO_PIN): value = (value-1)*4+2; break;  //第2列有按键被按下
		case (Col1_GPIO_PIN | Col2_GPIO_PIN | Col4_GPIO_PIN): value = (value-1)*4+3; break;  //第3列有按键被按下
		case (Col1_GPIO_PIN | Col2_GPIO_PIN | Col3_GPIO_PIN): value = (value-1)*4+4; break;  //第4列有按键被按下
		default: key_state=KEY_NULL;return 0; //没有按键按下，结束扫描函数
	}
	if(key_state==KEY_RELEASE)	return 0;//键值取完之后，到释放状态，不再取其他值
	if(key_state==KEY_NULL)	key_state=KEY_TOUCH;//没有按键按下的状态下  变成有按键接触的状态，但是要去抖动
	if(key_state<KEY_DEBOUNCE)	return 0;//按键去抖动，20ms
	else
	{
		key_state=KEY_PRESS;				//通过去抖动的20ms，则判断为有按键按下
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
			
			default: return 0; //按下的不是上述按键，就当是没有按键
		}
	}
}


