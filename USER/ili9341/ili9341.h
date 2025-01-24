#ifndef __ILI9341_H__
#define __ILI9341_H__

#include <stdint.h>
#include "main.h"             
#include "spi.h"              
#include "gpio.h"             
#include "stm32f4xx_hal.h"  
#include "delay.h"

#define USE_DMA_FillBlock 1	//�Ƿ�ʹ��DMA��������

extern uint8_t lcd_buf[];     // �����ⲿ����

//LCD��Ҫ������
typedef struct  
{										    
	uint16_t width;			//LCD ���
	uint16_t height;			//LCD �߶�
	uint16_t id;				//LCD ID
	uint8_t  dir;			//���������������ƣ�0��������1��������	
	uint16_t	 wramcmd;		//��ʼдgramָ��
	uint16_t  setxcmd;		//����x����ָ��
	uint16_t  setycmd;		//����y����ָ��	 
}_lcd_dev; 	

//LCD����
extern _lcd_dev lcddev;	//����LCD��Ҫ����

#define USE_HORIZONTAL  	 0//����Һ����˳ʱ����ת���� 	0-0����ת��1-90����ת��2-180����ת��3-270����ת

// LCD�ߴ綨��
#define LCD_WIDTH   240     // LCD���
#define LCD_HEIGHT  320     // LCD�߶�

// ����GPIO�˿ں�����
#define LCD_RST_GPIO_Port    GPIOB
#define LCD_RST_Pin         GPIO_PIN_12
#define LCD_DC_GPIO_Port     GPIOB    
#define LCD_DC_Pin          GPIO_PIN_14
#define LCD_CS_GPIO_Port     GPIOB
#define LCD_CS_Pin          GPIO_PIN_15
#define LCD_LED_GPIO_Port     GPIOB
#define LCD_LED_Pin          GPIO_PIN_13


// �����źź궨��
#define LCD_RST_Clr()   HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET)
#define LCD_RST_Set()   HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET)
                                              
#define LCD_DC_Clr()    HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET)
#define LCD_DC_Set()    HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET)
                                              
#define LCD_CS_Clr()    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET)
#define LCD_CS_Set()    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET)
                                              
#define LCD_LED_ON()    HAL_GPIO_WritePin(LCD_LED_GPIO_Port, LCD_LED_Pin, GPIO_PIN_SET)
#define LCD_LED_OFF()   HAL_GPIO_WritePin(LCD_LED_GPIO_Port, LCD_LED_Pin, GPIO_PIN_RESET)


//������ɫ
#define WHITE       0xFFFF
#define BLACK      	0x0000	  
#define BLUE       	0x001F  
#define BRED        0XF81F
#define GRED 			 	0XFFE0
#define GBLUE			 	0X07FF
#define RED         0xF800
#define MAGENTA     0xF81F
#define GREEN       0x07E0
#define CYAN        0x7FFF
#define YELLOW      0xFFE0
#define BROWN 			0XBC40 //��ɫ
#define BRRED 			0XFC07 //�غ�ɫ
#define GRAY  			0X8430 //��ɫ
//GUI��ɫ
#define DARKBLUE      	 0X01CF	//����ɫ
#define LIGHTBLUE      	 0X7D7C	//ǳ��ɫ  
#define GRAYBLUE       	 0X5458 //����ɫ
//������ɫΪPANEL����ɫ 
 
#define LIGHTGREEN     	0X841F //ǳ��ɫ
#define LIGHTGRAY     0XEF5B //ǳ��ɫ(PANNEL)
#define LGRAY 			 		0XC618 //ǳ��ɫ(PANNEL),���屳��ɫ
#define LGRAYBLUE      	0XA651 //ǳ����ɫ(�м����ɫ)
#define LBBLUE          0X2B12 //ǳ����ɫ(ѡ����Ŀ�ķ�ɫ)

void LCD_Init(void);
void LCD_Clear(uint16_t color);
void LCD_BackLight_On(void);
void LCD_BackLight_Off(void);

void LCD_FillBlock(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t *color);

void LCD_DrawPoint(uint16_t startX, uint16_t startY, uint16_t color);
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void LCD_DrawFilledCircle(uint16_t centerX, uint16_t centerY, uint16_t radius, uint16_t color);

#endif // __LCD_H__
