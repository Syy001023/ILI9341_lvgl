#ifndef __TOUCH_H__
#define __TOUCH_H__

#include <stdint.h>
#include "main.h"             
#include "spi.h"              
#include "gpio.h"             
#include "stm32f4xx_hal.h"    

//是否使用校准程序，我没有AT24C02，所以直接赋经验值了
#define USE_TP_ADJUST 0

// 触摸屏状态
#define TP_PRESS_DOWN    0x80  // 触屏被按下
#define TP_PRESS_LIFT    0x40  // 触屏被按下后松开

// 触摸屏控制器结构体
typedef struct {
    uint8_t (*init)(void);     // 初始化触摸屏控制器
    uint8_t (*scan)(uint8_t);  // 扫描触摸屏
    void (*adjust)(void);      // 触摸屏校准
    uint16_t x0;              // 原始坐标(第一次按下时的坐标)
    uint16_t y0;
    uint16_t x;               // 当前坐标(此次扫描时的坐标)
    uint16_t y;
    uint8_t sta;             // 笔的状态
    float xfac;              // 触摸屏校准参数
    float yfac;
    short xoff;
    short yoff;
    uint8_t touchtype;       // 触摸屏类型
} TouchTypeDef;

extern TouchTypeDef tp_dev;

// 触摸屏IO定义
#define TOUCH_PEN_PIN       GPIO_PIN_1   // T_PEN   (PB1)
#define TOUCH_PEN_PORT      GPIOB
#define TOUCH_MISO_PIN      GPIO_PIN_2   // T_MISO  (PB2)
#define TOUCH_MISO_PORT     GPIOB
#define TOUCH_MOSI_PIN      GPIO_PIN_11  // T_MOSI  (PF11)
#define TOUCH_MOSI_PORT     GPIOF
#define TOUCH_CLK_PIN       GPIO_PIN_0   // T_SCK   (PB0)
#define TOUCH_CLK_PORT      GPIOB
#define TOUCH_CS_PIN        GPIO_PIN_5   // T_CS    (PC5)
#define TOUCH_CS_PORT       GPIOC

// IO操作函数
#define PEN_READ()   HAL_GPIO_ReadPin(TOUCH_PEN_PORT, TOUCH_PEN_PIN)
#define DOUT_READ()  HAL_GPIO_ReadPin(TOUCH_MISO_PORT, TOUCH_MISO_PIN)
#define TDIN(n)      HAL_GPIO_WritePin(TOUCH_MOSI_PORT, TOUCH_MOSI_PIN, ((n) ? GPIO_PIN_SET : GPIO_PIN_RESET))
#define TCLK(n)      HAL_GPIO_WritePin(TOUCH_CLK_PORT, TOUCH_CLK_PIN, ((n) ? GPIO_PIN_SET : GPIO_PIN_RESET))
#define TCS(n)       HAL_GPIO_WritePin(TOUCH_CS_PORT, TOUCH_CS_PIN, ((n) ? GPIO_PIN_SET : GPIO_PIN_RESET))

// 函数声明
void TP_Write_Byte(uint8_t num);
uint16_t TP_Read_AD(uint8_t cmd);
uint16_t TP_Read_XOY(uint8_t xy);
uint8_t TP_Read_XY(uint16_t *x, uint16_t *y);
uint8_t TP_Read_XY2(uint16_t *x, uint16_t *y);
void TP_Drow_Touch_Point(uint16_t x, uint16_t y, uint16_t color);
void TP_Draw_Big_Point(uint16_t x, uint16_t y, uint16_t color);
uint8_t TP_Scan(uint8_t tp);
void TP_Save_Adjdata(void);
uint8_t TP_Get_Adjdata(void);
void TP_Adjust(void);
uint8_t TP_Init(void);
void TP_Adj_Info_Show(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, 
                      uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t fac);

void Touch_Test(void);

#endif
