#include "lcd.h"
#include "systick.h"
#include "font.h"

/**
 *	LCD���Դ�
 *  ��Ÿ�ʽ����.
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
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;											//����Ϊ�������
	GPIO_InitStruct.GPIO_Pin = LCD_SDA_GPIO_PIN | LCD_SCK_GPIO_PIN | LCD_RS_GPIO_PIN | LCD_RST_GPIO_PIN | LCD_CS_GPIO_PIN ;		//ѡ��pin
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_40MHz;									//�ٶ�50M
	GPIO_Init(GPIOB, &GPIO_InitStruct);														//��ʼ��GPIO
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;											//����Ϊ�������
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP; 
	GPIO_InitStruct.GPIO_Pin = LCD_BL_GPIO_PIN ;										//ѡ��pin
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_40MHz;									//�ٶ�50M
	GPIO_Init(GPIOD, &GPIO_InitStruct);														//��ʼ��GPIO
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

 //************��ʾ��***************//
void  LCD_Disp_On(void)
{ 
//	LCD_WriteCommand(0xA4);//�ر���ʾȫ������
//	LCD_WriteCommand(0xAD);//��̬ͼ����ʾ ��
//	LCD_WriteData(0x03);		//����
	LCD_WriteCommand(0xaf);	//����ʾ
}
 //************��ʾ��***************//
void  LCD_Disp_Off(void)
{ 
//	LCD_WriteCommand(0xAC);	//��̬ͼ����ʾ ��
//	LCD_WriteData(0x00);		//��̬ͼ����ʾ ��
	LCD_WriteCommand(0xae);	//����ʾ
//	LCD_WriteCommand(0xa5);	//��ʾȫ������
}
 //************����IC��ʼ��***************//
void  LCD_Ini (void)
{ 
	LCD_RST_LOW; 
	SysTick_Delay_Ms(100);		//Delay 1ms 
	LCD_RST_HIGH; 
	SysTick_Delay_Ms(20);		//Delay 120ms 
	
	LCD_WriteCommand(0xE2); //�����λ
	SysTick_Delay_Ms(5); //Delay 5ms
	LCD_WriteCommand(0x2c);//��ѹ����1
	SysTick_Delay_Ms(5);
	LCD_WriteCommand(0x2e);//��ѹ����2
	SysTick_Delay_Ms(5);
	LCD_WriteCommand(0x2f);//��ѹ����3
	SysTick_Delay_Ms(5);
	LCD_WriteCommand(0x23);//�ֵ��Աȶȣ������÷�Χ0x20~0x27
	LCD_WriteCommand(0x81);//΢���Աȶ�
	LCD_WriteCommand(0x28);//΢���Աȶȣ������÷�Χ0x00~0x3f
	LCD_WriteCommand(0xa2);//1/9ƫѹ�ȣ�bias��
	LCD_WriteCommand(0xc0);//��ɨ��˳�� ���ϵ���
	LCD_WriteCommand(0xa1);//��ɨ��˳�� ������
	LCD_WriteCommand(0x40);//��ʼ�У��ӵ�һ�п�ʼ
	LCD_WriteCommand(0xaf);//����ʾ
	
	//LCD_Fill_All(0x00);
	LCD_Refresh_Gram();
}

void LCD_SetPosition(uint8_t x_pos , uint8_t page)  //x=0-127,page=0-7
{
	page = page%8;    //ҳ������7�����ص���0ҳ
	LCD_WriteCommand(0xB0+page);
	LCD_WriteCommand(((x_pos&0xf0)>>4) | 0x10);
	LCD_WriteCommand((x_pos&0x0f)| 0x04); 
}

//�����Դ浽LCD
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
  * @brief  ����
  * @param	x: 0~127	y: 0~63		t:1 ��� 0 ���
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
void LCD_DrawPoint1(u8 x, u8 y, u8 dot)//�����ε�ʱ����д��λ
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
	for (i = x0 ; i <= x1; i++)	//���λ���
	{
		LCD_DrawPoint1(i, y0, t);
		LCD_DrawPoint1(i, y1, t);
	}
	
	for (i = y0 ; i <= y1; i++)	//���λ���
	{
		LCD_DrawPoint1(x0, i, t);
		LCD_DrawPoint1(x1, i, t);
	}
}

/** 
  * @name	void LCD_Fill(u8 x0, u8 y0, u8 x1, u8 y1, u8 dot)
  * @brief  �������
  * @param	x0,y0,x1,y1 �������ĶԽ�����
  *			ȷ��x0<=x1;y0<=y1 0<=x0<=127 0<=y0<=63
  *			dot:0,���;1,���
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

//�����Ļ0x00����0xff
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
	LCD_Refresh_Gram();//������ʾ
}

//�����Ļ0x00����0xff
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
	//LCD_Refresh_Gram();//������ʾ
}
	
//page=0123  x_pos=0-7
void LCD_Show_ASCII(u8 page , u8 x_pos , char *str, TextSize textsize,u8 mode)
{
	uint16_t ascii_num = 0; 	//�ַ�ת����ASCII������ֿ��������ľ���λ�ã�
	uint8_t t,i = 0;   				//ѭ��������һ���ַ���8���ֽ���ʾ������Ҫ8��д���ֽڣ�
	u16 s_y,y,x,temp;
																																														
	switch(textsize)
	{		
		case font_6x12:  //6x12����
		{
			y  = page*10;
			s_y = y;
			x=x_pos*6;
			while ( *str != '\0' )
			{
				ascii_num = *str -32;  	//ȡ���±ꣻ
				ascii_num *= 12;
				for (t = 0; t <6; t++)
				{   
					temp = (ASCII_6x12[ascii_num]<< 8) | ASCII_6x12[ascii_num+1];  //һ����д��������,����Ҫ����ȡģ���
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
						s_y+=12;//ÿ���ַ���Ҫ��ҳ��2���ֽڣ�
						y = s_y;
					}
					ascii_num+=2;	
				}
				str ++; //�¸��ַ�
			}
		}
		break;		
		case font_8x16:  //8x16����
		{
			y  = page*16;
			s_y = y;
			x=x_pos*8;
			while ( *str != '\0' )
			{
				ascii_num = *str -32;  	//ȡ���±ꣻ
				ascii_num *= 16;
				for (t = 0; t <8; t++)
				{
					temp = (ASCII_8x16[ascii_num] << 8) | ASCII_8x16[ascii_num+1];  //һ����д��������
 
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
						s_y+=16;//ÿ���ַ���Ҫ��ҳ��2���ֽڣ�
						if(s_y>63)
							s_y=0;
						y = s_y;
					}
					ascii_num+=2;	
				}
				str ++; //�¸��ַ�
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
//		HZ_h=*str;	HZ_l=*(++str);	//��ù�����ĸߵ��ֽ�
//		HZ_h-=0xA0;	HZ_l-=0xA0;
//		HZ_addr=(HZ_h*94+HZ_l-95)*32;//��ø������ַ��������ֿ��е�λ��
//	
//		SPI_FLASH_BufferRead(HZ_FONT_BUF, HZ_addr, 32);		
//		for (t = 0; t <16; t++)
//		{
//			temp = (HZ_FONT_BUF[b_index]<< 8) | HZ_FONT_BUF[b_index+1];  //һ����д��������,����Ҫ����ȡģ���
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
//				s_y+=16;//ÿ���ַ���Ҫ��ҳ��2���ֽڣ� 
//				y = s_y;
//			}
//			b_index+=2;	
//		}
//		str ++; //�¸��ַ�
//		b_index=0;
//	}
//}

void LCD_ShowCN(u8 page, u8 x_pos, u8 pos, u8 len, u8 mode)
{
	u16 temp, t, i;
	u16 s_y,y,x;
	u16 adder = pos * 32;	//�õ�����һ���ַ���Ӧ������ռ���ֽ���		
	u16 csize = len * 16;	//һ������д16�Σ�ÿ��д�������ݣ�����ȡ��
	
	y  = page*16;
	s_y = y;
	x=x_pos*16;
	
	for (t = 0; t < csize; t++)
	{
		temp = (CN_16x16[adder] << 8) | CN_16x16[adder+1];  //һ����д��������
 
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
			s_y+=16;//ÿ��������Ҫ��ҳ��2���ֽڣ�
			y = s_y;
		}
		adder+=2;	
	}
	//OLED_Refresh_Gram();
}

//��ʾСͼ�꺯������С16*16
void LCD_ShowIcon(u8 page, u8 x_pos, u8 mode, u8 Picture_Index)
{
	u16 temp, t, i;
	u16 s_y,y,s_x,x;
	u32 count = Picture_Index*32; //��¼�ڼ���ͼƬ����ʼ�±�
	
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

	//128*64 ͼƬģʽ��ʾ������
	/*
		( x0 , y0 ) ��ʼ���꣬x0: ��ʼ��(1~8)   y0: ��ʼ��(0~128);
		( x1 , y1 ) �յ����꣬x1: �յ���(1~8)   y1: �յ���(0~128);
		���и��ݹ淶ʹ�ã������淶ʹ��ʱû���ṩ�������ܣ�
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
 
 
 
 
	
	
 
	
	
	
