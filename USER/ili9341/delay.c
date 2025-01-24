#include "tim.h"
#include "delay.h"

void delay_us(uint16_t us)
{
    __HAL_TIM_SET_COUNTER(&htim1, 0);  // ������������
    HAL_TIM_Base_Start(&htim1);        // ������ʱ��
    while (__HAL_TIM_GET_COUNTER(&htim1) < us)  // �ȴ�ֱ���������ﵽָ��ֵ
    {
    }
    HAL_TIM_Base_Stop(&htim1);         // ֹͣ��ʱ��
}

void delay_ms(uint16_t ms)
{
    for(uint16_t i = 0; i < ms; i++)
    {
        delay_us(1000);
    }
}
