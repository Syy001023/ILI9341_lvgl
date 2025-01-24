#include "tim.h"
#include "delay.h"

void delay_us(uint16_t us)
{
    __HAL_TIM_SET_COUNTER(&htim1, 0);  // 将计数器清零
    HAL_TIM_Base_Start(&htim1);        // 启动定时器
    while (__HAL_TIM_GET_COUNTER(&htim1) < us)  // 等待直到计数器达到指定值
    {
    }
    HAL_TIM_Base_Stop(&htim1);         // 停止定时器
}

void delay_ms(uint16_t ms)
{
    for(uint16_t i = 0; i < ms; i++)
    {
        delay_us(1000);
    }
}
