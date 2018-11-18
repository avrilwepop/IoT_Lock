#include "lcd.h"
#include "systick.h"
#include "font.h"

/**
 *	LCD的显存
 *  存放格式如下.
 *  [0]0 1 2 3 ... 127	
 *  [1]0 1 2 3 ... 127
 *  [2]0 1 2 3 ... 127
 *  [3]0 1 2 3 ... 127
 *  [4]0 1 2 3 ... 127
 *  [5]0 1 2 3 ... 127
 *  [6]0 1 2 3 ... 127
 *  [7]0 1 2 3 ... 127 
**/	   

unsigned char LCD_GRAM[128][8];

void LCD_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;											//配置为推挽输出
	GPIO_InitStruct.GPIO_Pin = LCD_SDA_GPIO_PIN | LCD_SCK_GPIO_PIN | LCD_RS_GPIO_PIN | LCD_RST_GPIO_PIN | LCD_CS_GPIO_PIN ;		//选择pin
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_40MHz;									//速度50M
	GPIO_Init(GPIOB, &GPIO_InitStruct);														//初始化GPIO
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;											//配置为推挽输出
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP; 
	GPIO_InitStruct.GPIO_Pin = LCD_BL_GPIO_PIN ;										//选择pin
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_40MHz;									//速度50M
	GPIO_Init(GPIOD, &GPIO_InitStruct);														//初始化GPIO
	LCD_BL_ON;
}

//**********write register command***********//
void  LCD_WriteCommand(uint8_t com)
{
	uint8_t i;

	LCD_RS_COMM;
	LCD_CS_LOW;
	for(i=0;i<8;i++)
	{
		LCD_SCK_LOW;
		if(com & 0x80)
			LCD_SDA_HIGH;
		else 	
			LCD_SDA_LOW;
		LCD_SCK_HIGH;
		com <<=1;
	}
	LCD_CS_HIGH;	
	LCD_RS_DATA;
}
   
//***************write  data****************// 
void LCD_WriteData(uint8_t dat)
{
	uint8_t i;
	
	LCD_RS_DATA;;
	LCD_CS_LOW;;
	for(i=0;i<8;i++)
	{
		LCD_SCK_LOW;
		if(dat & 0x80)
			LCD_SDA_HIGH;
		else 
			LCD_SDA_LOW;
		LCD_SCK_HIGH;
		dat<<=1;
	}
	LCD_CS_HIGH;
	LCD_RS_DATA;
}

 //************显示开***************//
void  LCD_Disp_On(void)
{ 
//	LCD_WriteCommand(0xA4);//关闭显示全部点阵
//	LCD_WriteCommand(0xAD);//静态图标显示 开
//	LCD_WriteData(0x03);		//数据
	LCD_WriteCommand(0xaf);	//开显示
}
 //************显示关***************//
void  LCD_Disp_Off(void)
{ 
//	LCD_WriteCommand(0xAC);	//静态图标显示 关
//	LCD_WriteData(0x00);		//静态图标显示 关
	LCD_WriteCommand(0xae);	//关显示
//	LCD_WriteCommand(0xa5);	//显示全部点阵
}
 //************驱动IC初始化***************//
void  LCD_Ini (void)
{ 
	LCD_RST_LOW; 
	SysTick_Delay_Ms(100);		//Delay 1ms 
	LCD_RST_HIGH; 
	SysTick_Delay_Ms(20);		//Delay 120ms 
	
	LCD_WriteCommand(0xE2); //软件复位
	SysTick_Delay_Ms(5); //Delay 5ms
	LCD_WriteCommand(0x2c);//升压步骤1
	SysTick_Delay_Ms(5);
	LCD_WriteCommand(0x2e);//升压步骤2
	SysTick_Delay_Ms(5);
	LCD_WriteCommand(0x2f);//升压步骤3
	SysTick_Delay_Ms(5);
	LCD_WriteCommand(0x23);//粗调对比度，可设置范围0x20~0x27
	LCD_WriteCommand(0x81);//微调对比度
	LCD_WriteCommand(0x28);//微调对比度，可设置范围0x00~0x3f
	LCD_WriteCommand(0xa2);//1/9偏压比（bias）
	LCD_WriteCommand(0xc0);//行扫描顺序 从上到下
	LCD_WriteCommand(0xa1);//列扫描顺序 从左到右
	LCD_WriteCommand(0x40);//起始行，从第一行开始
	LCD_WriteCommand(0xaf);//开显示
	
	//LCD_Fill_All(0x00);
	LCD_Refresh_Gram();
}

void LCD_SetPosition(uint8_t x_pos , uint8_t page)  //x=0-127,page=0-7
{
	page = page%8;    //页数超过7，返回到第0页
	LCD_WriteCommand(0xB0+page);
	LCD_WriteCommand(((x_pos&0xf0)>>4) | 0x10);
	LCD_WriteCommand((x_pos&0x0f)| 0x04); 
}

//更新显存到LCD
void LCD_Refresh_Gram(void)
{
	u8 i, n;	

	for(i = 0; i < 8; i++)
	{
		LCD_SetPosition(0, i);
		for(n = 0; n < 128; n++)
		{
			LCD_WriteData(LCD_GRAM[n][i]);
		}
	}
}

/** 
  * @name	void LCD_DrawPoint(u8 x,u8 y,u8 t)
  * @brief  画点
  * @param	x: 0~127	y: 0~63		t:1 填充 0 清空
  */
void LCD_DrawPoint(u8 x, u8 y, u8 dot)
{
	u8 pos, bx, temp = 0;
	
	if(x > 127 || y > 63)
	{
		x=0;y=0;
	}
	pos = y/8;
	bx = y % 8;
	temp = 1 << (7 - bx);
	dot ? (LCD_GRAM[x][pos] |= temp) : (LCD_GRAM[x][pos] &= ~temp);   
}
void LCD_DrawPoint1(u8 x, u8 y, u8 dot)//画矩形的时候，先写低位
{
	u8 pos, bx, temp = 0;
	
	if(x > 127 || y > 63)
	{
		x=0;y=0;
	}
	pos = y/8;
	bx = y % 8;
	temp = 1 <<  bx;
	dot ? (LCD_GRAM[x][pos] |= temp) : (LCD_GRAM[x][pos] &= ~temp);   
}
void LCD_DrawRectangle(u8 x0, u8 y0, u8 x1, u8 y1, u8 t)
{
	 u8 i;
	for (i = x0 ; i <= x1; i++)	//依次画点
	{
		LCD_DrawPoint1(i, y0, t);
		LCD_DrawPoint1(i, y1, t);
	}
	
	for (i = y0 ; i <= y1; i++)	//依次画点
	{
		LCD_DrawPoint1(x0, i, t);
		LCD_DrawPoint1(x1, i, t);
	}
}

/** 
  * @name	void LCD_Fill(u8 x0, u8 y0, u8 x1, u8 y1, u8 dot)
  * @brief  填充区域
  * @param	x0,y0,x1,y1 填充区域的对角坐标
  *			确保x0<=x1;y0<=y1 0<=x0<=127 0<=y0<=63
  *			dot:0,清空;1,填充
  * @retval	none
  */
void LCD_Fill_Area(u8 x0, u8 y0, u8 x1, u8 y1, u8 dot)
{
	u8 x, y;
	for (x = x0; x < x1; x++)
	{
		for (y = y0; y < y1; y++)
		{
			LCD_DrawPoint(x, y, dot);
		}
	}
	//LCD_Refresh_Gram();
}

//填充屏幕0x00或者0xff
void LCD_Fill_All(u8 val)  
{  
	u8 i, n;  
	for(i = 0; i < 8; i++)
	{
		for(n = 0; n < 128; n++)
		{
			LCD_GRAM[n][i] = val;
		}
	}			
	LCD_Refresh_Gram();//更新显示
}

//填充屏幕0x00或者0xff
void LCD_Fill_All_nul(u8 val)  
{  
	u8 i, n;  
	for(i = 0; i < 8; i++)
	{
		for(n = 0; n < 128; n++)
		{
			LCD_GRAM[n][i] = val;
		}
	}			
	//LCD_Refresh_Gram();//更新显示
}
	
//page=0123  x_pos=0-7
void LCD_Show_ASCII(u8 page , u8 x_pos , char *str, TextSize textsize,u8 mode)
{
	uint16_t ascii_num = 0; 	//字符转换成ASCII码后在字库中码表里的具体位置；
	uint8_t t,i = 0;   				//循环变量，一个字符由8个字节显示，故需要8次写入字节；
	u16 s_y,y,x,temp;
																																														
	switch(textsize)
	{		
		case font_6x12:  //6x12字体
		{
			y  = page*10;
			s_y = y;
			x=x_pos*6;
			while ( *str != '\0' )
			{
				ascii_num = *str -32;  	//取得下标；
				ascii_num *= 12;
				for (t = 0; t <6; t++)
				{   
					temp = (ASCII_6x12[ascii_num]<< 8) | ASCII_6x12[ascii_num+1];  //一次性写两个数据,具体要分析取模情况
					for (i = 0; i < 12; i++)
					{
						(temp & 0x8000) ? LCD_DrawPoint1(x, y, mode) : LCD_DrawPoint1(x, y, !mode);
						temp <<= 1;
						y++;
					} 
					y = s_y;
					x++;
					if(x>127)
					{
						x=0;
						s_y+=12;//每个字符需要两页，2个字节；
						y = s_y;
					}
					ascii_num+=2;	
				}
				str ++; //下个字符
			}
		}
		break;		
		case font_8x16:  //8x16字体
		{
			y  = page*16;
			s_y = y;
			x=x_pos*8;
			while ( *str != '\0' )
			{
				ascii_num = *str -32;  	//取得下标；
				ascii_num *= 16;
				for (t = 0; t <8; t++)
				{
					temp = (ASCII_8x16[ascii_num] << 8) | ASCII_8x16[ascii_num+1];  //一次性写两个数据
 
					for (i = 0; i < 16; i++)
					{
						(temp & 0x8000) ? LCD_DrawPoint(x, y, mode) : LCD_DrawPoint(x, y, !mode);
						temp <<= 1;
						y++;
					} 
					y = s_y;
					x++;
					if(x>127)
					{
						x=0;
						s_y+=16;//每个字符需要两页，2个字节；
						if(s_y>63)
							s_y=0;
						y = s_y;
					}
					ascii_num+=2;	
				}
				str ++; //下个字符
			}
		}
		break;
		
		default:break;
	}
}

//void LCD_Show_CN(u8 page , u8 x_pos , char *str,u8 mode)
//{
//	uint8_t b_index,t,i,HZ_h,HZ_l;
//	u16 s_y,y,x,temp;
//	u32 HZ_addr;																																											
//	u8 HZ_FONT_BUF[32];
//	
//	y  = page*16;
//	s_y = y;
//	x=x_pos*16;
//	while ( *str != '\0' )
//	{
//		HZ_h=*str;	HZ_l=*(++str);	//获得国标码的高低字节
//		HZ_h-=0xA0;	HZ_l-=0xA0;
//		HZ_addr=(HZ_h*94+HZ_l-95)*32;//获得该中文字符在整个字库中的位置
//	
//		SPI_FLASH_BufferRead(HZ_FONT_BUF, HZ_addr, 32);		
//		for (t = 0; t <16; t++)
//		{
//			temp = (HZ_FONT_BUF[b_index]<< 8) | HZ_FONT_BUF[b_index+1];  //一次性写两个数据,具体要分析取模情况
//			for (i = 0; i < 16; i++)
//			{
//				(temp & 0x8000) ? LCD_DrawPoint(x, y, mode) : LCD_DrawPoint(x, y, !mode);
//				temp <<= 1;
//				y++;
//			} 
//			y = s_y;
//			x++;
//			if(x>127)
//			{
//				x=0;
//				s_y+=16;//每个字符需要两页，2个字节； 
//				y = s_y;
//			}
//			b_index+=2;	
//		}
//		str ++; //下个字符
//		b_index=0;
//	}
//}

void LCD_ShowCN(u8 page, u8 x_pos, u8 pos, u8 len, u8 mode)
{
	u16 temp, t, i;
	u16 s_y,y,x;
	u16 adder = pos * 32;	//得到字体一个字符对应点阵集所占的字节数		
	u16 csize = len * 16;	//一个汉字写16次，每次写两个数据，逐列取摸
	
	y  = page*16;
	s_y = y;
	x=x_pos*16;
	
	for (t = 0; t < csize; t++)
	{
		temp = (CN_16x16[adder] << 8) | CN_16x16[adder+1];  //一次性写两个数据
 
		for (i = 0; i < 16; i++)
		{
			(temp & 0x8000) ? LCD_DrawPoint(x, y, mode) : LCD_DrawPoint(x, y, !mode);
			temp <<= 1;
			y++;
		} 
		y = s_y;
		x++;
		if(x>127)
		{
			x=0;
			s_y+=16;//每个汉字需要两页，2个字节；
			y = s_y;
		}
		adder+=2;	
	}
	//OLED_Refresh_Gram();
}

//显示小图标函数，大小16*16
void LCD_ShowIcon(u8 page, u8 x_pos, u8 mode, u8 Picture_Index)
{
	u16 temp, t, i;
	u16 s_y,y,s_x,x;
	u32 count = Picture_Index*32; //记录第几幅图片的起始下标
	
	y		=	page*16;
	s_y =	y;
	x		=	x_pos*16;
	s_x = x;
	for (t = 0; t < 16; t++)
	{   
		temp = (icon_table[count]<<8) | icon_table[count+1];

		for (i = 0; i < 16; i++)
		{
			(temp & 0x8000) ? LCD_DrawPoint(x, y, mode) : LCD_DrawPoint(x, y, !mode);
			temp <<= 1;
			y++;
		}
		y = s_y;
		x++;
		count+=2;
	}
	s_y+=8;
	y = s_y;
	x = s_x;
}

	//128*64 图片模式显示函数；
	/*
		( x0 , y0 ) 初始坐标，x0: 起始行(1~8)   y0: 起始列(0~128);
		( x1 , y1 ) 终点坐标，x1: 终点行(1~8)   y1: 终点列(0~128);
		自行根据规范使用，不按规范使用时没有提供错误处理功能；
	*/
void LCD_Show_Logo(void)
{
	u32 temp;
	u8 i,t,x,y;
	u16 count = 0;

	for (t = 0; t < 128; t++)
	{   
		temp = (LOGO_128x64[count]<<24) | (LOGO_128x64[count+1]<<16) | (LOGO_128x64[count+2]<<8) | LOGO_128x64[count+3];
		for (i = 0; i < 32; i++)
		{
			(temp & 0x80000000) ? LCD_DrawPoint(x, y, 1) : LCD_DrawPoint(x, y, 0);
			temp <<= 1;
			y++;
		}
		count+=4;
		
		temp = (LOGO_128x64[count]<<24) | (LOGO_128x64[count+1]<<16) | (LOGO_128x64[count+2]<<8) | LOGO_128x64[count+3];
		for (i = 0; i < 32; i++)
		{
			(temp & 0x80000000) ? LCD_DrawPoint(x, y, 1) : LCD_DrawPoint(x, y, 0);
			temp <<= 1;
			y++;
		}
		y = 0;
		x++;
		count+=4;
	}
}
 
 
 
 
	
	
 
	
	
	
