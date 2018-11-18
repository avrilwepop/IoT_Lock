#ifndef __LCD_H
#define __LCD_H

#include "stm32l1xx.h"

//BL    PD2
//CS    PB3
//RST 	PB4
//CD    PB5
//SCK   PB6
//SDA   PB7
typedef enum _TEXT_SIZE
{
	font_6x12 = 1,
	font_8x16 = 2,
}TextSize;

#define LCD_BL_GPIO_PIN			GPIO_Pin_2
#define LCD_CS_GPIO_PIN			GPIO_Pin_3
#define LCD_RST_GPIO_PIN			GPIO_Pin_4
#define LCD_RS_GPIO_PIN			GPIO_Pin_5
#define LCD_SCK_GPIO_PIN			GPIO_Pin_6
#define LCD_SDA_GPIO_PIN			GPIO_Pin_7

#define LCD_BL_GPIO_PORT			GPIOD
#define LCD_CS_GPIO_PORT			GPIOB
#define LCD_RST_GPIO_PORT		GPIOB
#define LCD_RS_GPIO_PORT			GPIOB
#define LCD_SCK_GPIO_PORT		GPIOB
#define LCD_SDA_GPIO_PORT		GPIOB

#define LCD_BL_ON						LCD_BL_GPIO_PORT ->ODR |=  LCD_BL_GPIO_PIN
#define LCD_BL_OFF			  		LCD_BL_GPIO_PORT ->ODR &= ~LCD_BL_GPIO_PIN

#define LCD_RS_DATA					LCD_RS_GPIO_PORT ->ODR  |=  LCD_RS_GPIO_PIN   //Ñ¡ÔñÊý¾Ý¼Ä´æÆ÷
#define LCD_RS_COMM					LCD_RS_GPIO_PORT ->ODR  &= ~LCD_RS_GPIO_PIN 	 //Ñ¡ÔñÖ¸Áî¼Ä´æÆ÷

#define LCD_SCK_HIGH					LCD_SCK_GPIO_PORT ->ODR |=  LCD_SCK_GPIO_PIN
#define LCD_SCK_LOW			  	LCD_SCK_GPIO_PORT ->ODR &= ~LCD_SCK_GPIO_PIN

#define LCD_SDA_HIGH					LCD_SDA_GPIO_PORT ->ODR |=  LCD_SDA_GPIO_PIN
#define LCD_SDA_LOW			  	LCD_SDA_GPIO_PORT ->ODR &= ~LCD_SDA_GPIO_PIN

#define LCD_CS_HIGH					LCD_CS_GPIO_PORT ->ODR |=  LCD_CS_GPIO_PIN
#define LCD_CS_LOW			  		LCD_CS_GPIO_PORT ->ODR &= ~LCD_CS_GPIO_PIN

#define LCD_RST_HIGH					LCD_RST_GPIO_PORT ->ODR |=  LCD_RST_GPIO_PIN
#define LCD_RST_LOW			  	LCD_RST_GPIO_PORT ->ODR &= ~LCD_RST_GPIO_PIN

void LCD_Refresh_Gram(void);
void LCD_Fill_All_nul(u8 val);
void LCD_DrawRectangle(u8 x0, u8 y0, u8 x1, u8 y1, u8 t);
void LCD_SetPosition(uint8_t x_pos , uint8_t page) ;
void LCD_GPIO_Config(void);
void LCD_Show_CN(u8 page , u8 x_pos , char *str,u8 mode);
void LCD_Fill_Area(u8 x0, u8 y0, u8 x1, u8 y1, u8 dot);
void LCD_ShowCN(u8 page, u8 x_pos, u8 pos, u8 len, u8 mode);
void LCD_Ini(void);
void LCD_Fill_All(u8 val);
void LCD_Show_FONT_16x32(unsigned char page , unsigned char x_pos  , unsigned char Num) ;
void LCD_Show_Logo(void);
void LCD_ShowIcon(u8 page, u8 x_pos, u8 mode, u8 Picture_Index);
void LCD_Show_ASCII(u8 page , u8 x_pos , char *str, TextSize textsize,u8 mode);
void LCD_Show_Frame(void);
extern void SPI_FLASH_BufferRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);
void  LCD_Disp_On(void);
void  LCD_Disp_Off(void);

#endif /*__LCD_H*/

