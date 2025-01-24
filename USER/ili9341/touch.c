#include "touch.h"
#include "ili9341.h"
#include "24cxx.h"
#include <stdlib.h>
#include <math.h>
#include "delay.h"
#include <stdio.h>

TouchTypeDef tp_dev = {
    TP_Init,
    TP_Scan,
    TP_Adjust,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

// Ĭ��Ϊtouchtype=0������
static uint8_t CMD_RDX = 0XD0;
static uint8_t CMD_RDY = 0X90;

/**
 * @brief  ������д��1byte����
 * @param  num: Ҫд�������
 * @retval None
 */
void TP_Write_Byte(uint8_t num)
{
    uint8_t count;
    for(count = 0; count < 8; count++)
    {
        if(num & 0x80)
            TDIN(1);
        else
            TDIN(0);
        num <<= 1;
        TCLK(0);
        delay_us(1);
        TCLK(1);
    }
}

/**
 * @brief  ��ȡ��������ADֵ
 * @param  cmd: ��ȡ����
 * @retval ��ȡ����ADֵ
 */
uint16_t TP_Read_AD(uint8_t cmd)
{
    uint8_t count = 0;
    uint16_t Num = 0;
    TCLK(0);
    TDIN(0);
    TCS(0);
    TP_Write_Byte(cmd);
    delay_us(6);
    TCLK(0);
    delay_us(1);
    TCLK(1);
    delay_us(1);
    TCLK(0);
    for(count = 0; count < 16; count++)
    {
        Num <<= 1;
        TCLK(0);
        delay_us(1);
        TCLK(1);
        if(DOUT_READ()) Num++;
    }
    Num >>= 4;
    TCS(1);
    return(Num);
}

#define READ_TIMES 5
#define LOST_VAL 1

/**
 * @brief  ��ȡһ������ֵ
 * @param  xy: ָ�CMD_RDX/CMD_RDY��
 * @retval ��ȡ����ֵ
 */
uint16_t TP_Read_XOY(uint8_t xy)
{
    uint16_t i, j;
    uint16_t buf[READ_TIMES];
    uint16_t sum = 0;
    uint16_t temp;
    
    for(i = 0; i < READ_TIMES; i++) buf[i] = TP_Read_AD(xy);
    for(i = 0; i < READ_TIMES - 1; i++)
    {
        for(j = i + 1; j < READ_TIMES; j++)
        {
            if(buf[i] > buf[j])
            {
                temp = buf[i];
                buf[i] = buf[j];
                buf[j] = temp;
            }
        }
    }
    sum = 0;
    for(i = LOST_VAL; i < READ_TIMES - LOST_VAL; i++) sum += buf[i];
    temp = sum / (READ_TIMES - 2 * LOST_VAL);
    return temp;
}

/**
 * @brief  ��ȡx,y����
 * @param  x,y: ��ȡ��������ֵ
 * @retval 0-��ȡʧ�ܣ�1-��ȡ�ɹ�
 */
uint8_t TP_Read_XY(uint16_t *x, uint16_t *y)
{
    *x = TP_Read_XOY(CMD_RDX);
    *y = TP_Read_XOY(CMD_RDY);
    return 1;
}

#define ERR_RANGE 50

/**
 * @brief  ������ȡ���δ���ֵ���ж��Ƿ���Ч
 * @param  x,y: ��ȡ��������ֵ
 * @retval 0-��ȡʧ�ܣ�1-��ȡ�ɹ�
 */
uint8_t TP_Read_XY2(uint16_t *x, uint16_t *y)
{
    uint16_t x1, y1;
    uint16_t x2, y2;
    uint8_t flag;
    
    flag = TP_Read_XY(&x1, &y1);
    if(flag == 0) return 0;
    flag = TP_Read_XY(&x2, &y2);
    if(flag == 0) return 0;
    
    if(((x2 <= x1 && x1 < x2 + ERR_RANGE) || (x1 <= x2 && x2 < x1 + ERR_RANGE)) &&
       ((y2 <= y1 && y1 < y2 + ERR_RANGE) || (y1 <= y2 && y2 < y1 + ERR_RANGE)))
    {
        *x = (x1 + x2) / 2;
        *y = (y1 + y2) / 2;
        return 1;
    }
    else return 0;
}

/**
 * @brief  ɨ�败����״̬
 * @param  tp: 0-��Ļ����; 1-��������
 * @retval ��ǰ����״̬
 */
uint8_t TP_Scan(uint8_t tp)
{
    if(!PEN_READ())
    {
        if(tp)
        {
            TP_Read_XY2(&tp_dev.x, &tp_dev.y);
        }
        else if(TP_Read_XY2(&tp_dev.x, &tp_dev.y))
        {
            tp_dev.x = tp_dev.xfac * tp_dev.x + tp_dev.xoff;
            tp_dev.y = tp_dev.yfac * tp_dev.y + tp_dev.yoff;
        }
        if((tp_dev.sta & TP_PRESS_DOWN) == 0)
        {
            tp_dev.sta = TP_PRESS_DOWN | TP_PRESS_LIFT;
            tp_dev.x0 = tp_dev.x;
            tp_dev.y0 = tp_dev.y;
        }
    }
    else
    {
        if(tp_dev.sta & TP_PRESS_DOWN)
        {
            tp_dev.sta &= ~(1 << 7);
        }
        else
        {
            tp_dev.x0 = 0;
            tp_dev.y0 = 0;
            tp_dev.x = 0xffff;
            tp_dev.y = 0xffff;
        }
    }
    return tp_dev.sta & TP_PRESS_DOWN;
}

#define SAVE_ADDR_BASE 40

/**
 * @brief  ����У׼����
 */
void TP_Save_Adjdata(void)
{
    int32_t temp;
    
    temp = tp_dev.xfac * 100000000;
    AT24CXX_WriteLenByte(SAVE_ADDR_BASE, temp, 4);
    temp = tp_dev.yfac * 100000000;
    AT24CXX_WriteLenByte(SAVE_ADDR_BASE + 4, temp, 4);
    AT24CXX_WriteLenByte(SAVE_ADDR_BASE + 8, tp_dev.xoff, 2);
    AT24CXX_WriteLenByte(SAVE_ADDR_BASE + 10, tp_dev.yoff, 2);
    AT24CXX_WriteOneByte(SAVE_ADDR_BASE + 12, tp_dev.touchtype);
    temp = 0x0A;
    AT24CXX_WriteOneByte(SAVE_ADDR_BASE + 13, temp);
	
	printf("tp_dev.xfac = %f\r\n", tp_dev.xfac);
	printf("tp_dev.yfac = %f\r\n", tp_dev.yfac);
	printf("tp_dev.xoff = %d\r\n", tp_dev.xoff);
	printf("tp_dev.yoff = %d\r\n", tp_dev.yoff);
	printf("tp_dev.touchtype = %u\r\n", tp_dev.touchtype);
}

/**
 * @brief  �õ������У׼ֵ
 * @retval 1-��ȡ�ɹ���0-��ȡʧ��
 */
uint8_t TP_Get_Adjdata(void)
{
    int32_t tempfac;
    tempfac = AT24CXX_ReadOneByte(SAVE_ADDR_BASE + 13);
    if(tempfac == 0x0A)
    {
        tempfac = AT24CXX_ReadLenByte(SAVE_ADDR_BASE, 4);
        tp_dev.xfac = (float)tempfac / 100000000;
        tempfac = AT24CXX_ReadLenByte(SAVE_ADDR_BASE + 4, 4);
        tp_dev.yfac = (float)tempfac / 100000000;
        tp_dev.xoff = AT24CXX_ReadLenByte(SAVE_ADDR_BASE + 8, 2);
        tp_dev.yoff = AT24CXX_ReadLenByte(SAVE_ADDR_BASE + 10, 2);
        tp_dev.touchtype = AT24CXX_ReadOneByte(SAVE_ADDR_BASE + 12);
        if(tp_dev.touchtype)
        {
            CMD_RDX = 0X90;
            CMD_RDY = 0XD0;
        }
        else
        {
            CMD_RDX = 0XD0;
            CMD_RDY = 0X90;
        }
        return 1;
    }
    return 0;
}

//���Ի�ʮ�ֱ�
void TP_Drow_Touch_Point(uint16_t x,uint16_t y,uint16_t color)
{
	LCD_DrawLine(x-12,y,x+13,y, color);//����
	LCD_DrawLine(x,y-12,x,y+13, color);//����
	LCD_DrawPoint(x+1,y+1, color);
	LCD_DrawPoint(x-1,y+1, color);
	LCD_DrawPoint(x+1,y-1, color);
	LCD_DrawPoint(x-1,y-1, color);
}	

//��������
void TP_Draw_Big_Point(uint16_t x,uint16_t y,uint16_t color)
{	    
	LCD_DrawPoint(x,y, color);//���ĵ� 
	LCD_DrawPoint(x+1,y, color);
	LCD_DrawPoint(x,y+1, color);
	LCD_DrawPoint(x+1,y+1, color);	 	  	
}

/**
 * @brief  ������У׼
 */
void TP_Adjust(void)
{
    uint16_t pos_temp[4][2];
    uint8_t cnt = 0;
    uint16_t d1, d2;
    uint32_t tem1, tem2;
    double fac;
    uint16_t outtime = 0;

    printf("��ʼ����У׼.\r\n");
    
	TP_Drow_Touch_Point(20, 20, RED);
	printf("Please touch point1.\r\n");
    while(1)
    {
        tp_dev.scan(1);
        if((tp_dev.sta & 0xc0) == TP_PRESS_LIFT)
        {
            outtime = 0;
            tp_dev.sta &= ~(1 << 6);
            
            pos_temp[cnt][0] = tp_dev.x;
            pos_temp[cnt][1] = tp_dev.y;
            cnt++;
            
            switch(cnt)
            {
			case 0:

				break;
			case 1:
                    TP_Drow_Touch_Point(20, 20, WHITE); // �����1
                    TP_Drow_Touch_Point(lcddev.width - 20, 20, RED); // ����2
				printf("Please touch point2.\r\n");
                    break;
                case 2:
                    TP_Drow_Touch_Point(lcddev.width - 20, 20, WHITE); // �����2
                    TP_Drow_Touch_Point(20, lcddev.height - 20, RED); // ����3
				printf("Please touch point3.\r\n");
                    break;
                case 3:
                    TP_Drow_Touch_Point(20, lcddev.height - 20, WHITE); // �����3
                    TP_Drow_Touch_Point(lcddev.width - 20, lcddev.height - 20, RED); // ����4
				printf("Please touch point4.\r\n");
                    break;
                case 4:
                    // ��ʼ����
                    tem1 = abs(pos_temp[0][0] - pos_temp[1][0]);
                    tem2 = abs(pos_temp[0][1] - pos_temp[1][1]);
                    tem1 *= tem1;
                    tem2 *= tem2;
                    d1 = sqrt(tem1 + tem2);
                    
                    tem1 = abs(pos_temp[2][0] - pos_temp[3][0]);
                    tem2 = abs(pos_temp[2][1] - pos_temp[3][1]);
                    tem1 *= tem1;
                    tem2 *= tem2;
                    d2 = sqrt(tem1 + tem2);
                    fac = (float)d1 / d2;
                    
                    if(fac < 0.95 || fac > 1.05 || d1 == 0 || d2 == 0)
                    {
                        cnt = 0;
					TP_Drow_Touch_Point(lcddev.width - 20, lcddev.height - 20, WHITE);
					TP_Drow_Touch_Point(20, 20, RED); // ����1
                        printf("Calibration failed! Please try again!\r\n");
					printf("Please touch point1.\r\n");
                        continue;
                    }
                    
                    // ������
                    tp_dev.xfac = (float)(lcddev.width - 40) / (pos_temp[1][0] - pos_temp[0][0]);
                    tp_dev.xoff = (lcddev.width - tp_dev.xfac * (pos_temp[1][0] + pos_temp[0][0])) / 2;
                    
                    tp_dev.yfac = (float)(lcddev.height - 40) / (pos_temp[2][1] - pos_temp[0][1]);
                    tp_dev.yoff = (lcddev.height - tp_dev.yfac * (pos_temp[2][1] + pos_temp[0][1])) / 2;
                    
                    if(abs(tp_dev.xfac) > 2 || abs(tp_dev.yfac) > 2)
                    {
                        cnt = 0;
					TP_Drow_Touch_Point(lcddev.width - 20, lcddev.height - 20, WHITE);
					TP_Drow_Touch_Point(20, 20, RED); // ����1
					printf("Calibration failed! Please try again!\r\n");
					printf("Please touch point1.\r\n");
                        tp_dev.touchtype = !tp_dev.touchtype;
                        if(tp_dev.touchtype)
                        {
                            CMD_RDX = 0X90;
                            CMD_RDY = 0XD0;
                        }
                        else
                        {
                            CMD_RDX = 0XD0;
                            CMD_RDY = 0X90;
                        }
                        continue;
                    }
                    TP_Drow_Touch_Point(lcddev.width - 20, lcddev.height - 20, WHITE);
                    printf("Touch Screen Adjust OK!\r\n");
                    HAL_Delay(1000);
                    TP_Save_Adjdata();
                    return;
            }
        }
        HAL_Delay(10);
        outtime++;
        if(outtime > 1000)
        {
            TP_Get_Adjdata();
            break;
        }
    }
}

/**
 * @brief  ��������ʼ��
 * @retval 0-��ʼ��ʧ�ܣ�1-��ʼ���ɹ�
 */
uint8_t TP_Init(void){
#if USE_TP_ADJUST
    TP_Read_XY(&tp_dev.x, &tp_dev.y);
    AT24CXX_Init();
    
    if(TP_Get_Adjdata())
    {
        return 0;
    }
    else
    {
        TP_Adjust();
        TP_Save_Adjdata();
    }
#else
	tp_dev.xfac = -0.067454;
	tp_dev.yfac = -0.090615;
	tp_dev.xoff = 258;
	tp_dev.yoff = 357;
	tp_dev.touchtype = 0;
#endif
    return 1;
}


void Touch_Test(void)
{
	TP_Init();
    uint16_t x, y;
    uint8_t touchStatus;

    while (1)
    {
        // ʹ�� TP_Scan ������ⴥ��״̬
        touchStatus = TP_Scan(0); // 0 ��ʾ��ȡ��Ļ����

        if (touchStatus & TP_PRESS_DOWN) // ���������������
        {
            x = tp_dev.x;
            y = tp_dev.y;
//            printf("Touch detected at: x=%d, y=%d\r\n", x, y);

            // �ڴ��������һ�����
            TP_Draw_Big_Point(x, y, BLACK);
        }
        else
        {

        }
    }
}
