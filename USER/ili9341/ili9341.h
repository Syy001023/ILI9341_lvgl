#ifndef __ILI9341_H__
#define __ILI9341_H__

#include <stdint.h>
#include "main.h"             
#include "spi.h"              
#include "gpio.h"             
#include "stm32f4xx_hal.h"  
#include "delay.h"

#define USE_DMA_FillBlock 1	//是否使用DMA搬运数据

extern uint8_t lcd_buf[];     // 声明外部变量

//LCD重要参数集
typedef struct  
{										    
	uint16_t width;			//LCD 宽度
	uint16_t height;			//LCD 高度
	uint16_t id;				//LCD ID
	uint8_t  dir;			//横屏还是竖屏控制：0，竖屏；1，横屏。	
	uint16_t	 wramcmd;		//开始写gram指令
	uint16_t  setxcmd;		//设置x坐标指令
	uint16_t  setycmd;		//设置y坐标指令	 
}_lcd_dev; 	

//LCD参数
extern _lcd_dev lcddev;	//管理LCD重要参数

#define USE_HORIZONTAL  	 0//定义液晶屏顺时针旋转方向 	0-0度旋转，1-90度旋转，2-180度旋转，3-270度旋转

// LCD尺寸定义
#define LCD_WIDTH   240     // LCD宽度
#define LCD_HEIGHT  320     // LCD高度

// 定义GPIO端口和引脚
#define LCD_RST_GPIO_Port    GPIOB
#define LCD_RST_Pin         GPIO_PIN_12
#define LCD_DC_GPIO_Port     GPIOB    
#define LCD_DC_Pin          GPIO_PIN_14
#define LCD_CS_GPIO_Port     GPIOB
#define LCD_CS_Pin          GPIO_PIN_15
#define LCD_LED_GPIO_Port     GPIOB
#define LCD_LED_Pin          GPIO_PIN_13


// 控制信号宏定义
#define LCD_RST_Clr()   HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET)
#define LCD_RST_Set()   HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET)
                                              
#define LCD_DC_Clr()    HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET)
#define LCD_DC_Set()    HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET)
                                              
#define LCD_CS_Clr()    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET)
#define LCD_CS_Set()    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET)
                                              
#define LCD_LED_ON()    HAL_GPIO_WritePin(LCD_LED_GPIO_Port, LCD_LED_Pin, GPIO_PIN_SET)
#define LCD_LED_OFF()   HAL_GPIO_WritePin(LCD_LED_GPIO_Port, LCD_LED_Pin, GPIO_PIN_RESET)


//画笔颜色
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
#define BROWN 			0XBC40 //棕色
#define BRRED 			0XFC07 //棕红色
#define GRAY  			0X8430 //灰色
//GUI颜色
#define DARKBLUE      	 0X01CF	//深蓝色
#define LIGHTBLUE      	 0X7D7C	//浅蓝色  
#define GRAYBLUE       	 0X5458 //灰蓝色
//以上三色为PANEL的颜色 
 
#define LIGHTGREEN     	0X841F //浅绿色
#define LIGHTGRAY     0XEF5B //浅灰色(PANNEL)
#define LGRAY 			 		0XC618 //浅灰色(PANNEL),窗体背景色
#define LGRAYBLUE      	0XA651 //浅灰蓝色(中间层颜色)
#define LBBLUE          0X2B12 //浅棕蓝色(选择条目的反色)

void LCD_Init(void);
void LCD_Clear(uint16_t color);
void LCD_BackLight_On(void);
void LCD_BackLight_Off(void);

void LCD_FillBlock(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t *color);

void LCD_DrawPoint(uint16_t startX, uint16_t startY, uint16_t color);
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void LCD_DrawFilledCircle(uint16_t centerX, uint16_t centerY, uint16_t radius, uint16_t color);

#endif // __LCD_H__
