#include "ili9341.h"
#include "spi.h"
#include "dma.h"

_lcd_dev lcddev;

/**
 * @brief  �������ST7789��ʾ��
 * @param  cmd: Ҫ���͵������ֽ�
 * @note   ��������ʱ��DC������Ҫ�õͣ�CS�������õͺ��ø�
 * @retval None
 */
void LCD_SendCmd(uint8_t cmd)
{
    LCD_DC_Clr();    // ����ģʽ
    LCD_CS_Clr();    // Ƭѡʹ��
    
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    
    LCD_CS_Set();    // Ƭѡ����
}

/**
 * @brief  �������ݵ�ST7789��ʾ��
 * @param  data: Ҫ���͵������ֽ�
 * @note   ��������ʱ��DC������Ҫ�øߣ�CS�������õͺ��ø�
 * @retval None
 */
void LCD_SendData(uint8_t data)
{
    LCD_DC_Set();    // ����ģʽ
    LCD_CS_Clr();    // Ƭѡʹ��
    
    HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
    
    LCD_CS_Set();    // Ƭѡ����
}

/**
 * @brief  ��LCDд��Ĵ���������
 * @param  reg: �Ĵ�����ַ
 * @param  data: Ҫд�������
 * @retval None
 */
void LCD_WriteReg(uint8_t reg, uint16_t data)
{
    LCD_SendCmd(reg);
    LCD_SendData(data);
}

/**
 * @brief  ׼����LCD GRAMд������
 * @note   ��д����������֮ǰ��Ҫ���ô˺���
 * @param  None
 * @retval None
 */
void LCD_WriteRAM_Prepare(void)
{
    LCD_SendCmd(lcddev.wramcmd);
}

/**
 * @brief  ��LCDд��16λ����
 * @param  data: Ҫд���16λ����
 * @retval None
 */
void LCD_WriteData_16Bit(uint16_t data)
{
    LCD_DC_Set();    // ����ģʽ
    LCD_CS_Clr();    // Ƭѡʹ��
    
    // �ȷ��͸�8λ���ٷ��͵�8λ
    HAL_SPI_Transmit(&hspi1, (uint8_t*)&data + 1, 1, HAL_MAX_DELAY);
    HAL_SPI_Transmit(&hspi1, (uint8_t*)&data, 1, HAL_MAX_DELAY);
    
    LCD_CS_Set();    // Ƭѡ����
}

void lcd_write_16bit_data_array(const uint16_t *data, uint32_t len)
{
    uint16_t i;
    LCD_CS_Clr();
    LCD_DC_Set(); 
    for (i = 0; i < len; i++)
    {
        uint8_t high_byte = data[i] >> 8;        // ��ȡ��8λ
        uint8_t low_byte = data[i] & 0xFF;       // ��ȡ��8λ
        
        HAL_SPI_Transmit(&hspi1, &high_byte, 1, HAL_MAX_DELAY);
        HAL_SPI_Transmit(&hspi1, &low_byte, 1, HAL_MAX_DELAY);
    }
    LCD_CS_Set();
}


void LCD_RESET(void)
{
	LCD_RST_Clr();
	delay_ms(50);
	LCD_RST_Set();
	delay_ms(50);
}

void LCD_BackLight_On(void){
	HAL_GPIO_WritePin(LCD_LED_GPIO_Port, LCD_LED_Pin, GPIO_PIN_SET);
}

void LCD_BackLight_Off(void){
	HAL_GPIO_WritePin(LCD_LED_GPIO_Port, LCD_LED_Pin, GPIO_PIN_RESET);
}

/**
 * @brief  ����LCD��ʾ����
 * @details ������ʾ���ں��������������ɫ���ݣ������ظ���������
 * @param  startX: �������x������
 * @param  startY: �������y������
 * @param  width:  ���ڿ��
 * @param  height: ���ڸ߶�
 * @note   ������ʾ�������Զ�����д��ģʽ(RAMWR)
 * @retval None
 */
void LCD_SetWindow(uint16_t startX, uint16_t startY, uint16_t endX, uint16_t endY)
{
    // �����е�ַ��Χ
    LCD_SendCmd(0x2A);        // Column Address Set
    LCD_SendData(startX >> 8);     // ��ʼ�е�ַ��8λ
    LCD_SendData(startX & 0xFF);   // ��ʼ�е�ַ��8λ
    LCD_SendData(endX >> 8);       // �����е�ַ��8λ
    LCD_SendData(endX & 0xFF);     // �����е�ַ��8λ

    // �����е�ַ��Χ
    LCD_SendCmd(0x2B);        // Row Address Set
    LCD_SendData(startY >> 8);     // ��ʼ�е�ַ��8λ
    LCD_SendData(startY & 0xFF);   // ��ʼ�е�ַ��8λ
    LCD_SendData(endY >> 8);       // �����е�ַ��8λ
    LCD_SendData(endY & 0xFF);     // �����е�ַ��8λ
    
    // ��ʼд���Դ�
    LCD_SendCmd(0x2C);        // Memory Write
}

/**
 * @brief  ����LCD���λ��
 * @param  x: x����
 * @param  y: y����
 * @retval None
 */
void LCD_SetCursor(uint16_t x, uint16_t y)
{
    LCD_SetWindow(x, y, x, y);
}

/**
 * @brief  ʹ��ָ����ɫ���������Ļ
 * @param  color: 16λRGB565��ʽ����ɫֵ
 * @note   ʹ������д��ģʽ�������
 * @retval None
 */
void LCD_Clear(uint16_t color)
{
    uint32_t total_pixels = lcddev.width * lcddev.height;
    uint8_t color_high = color >> 8;
    uint8_t color_low = color & 0xFF;
    
    // ����ȫ����ʾ����
    LCD_SetWindow(0, 0, lcddev.width, lcddev.height);
    
    // ����������ɫ����
    LCD_DC_Set();    // ����ģʽ
    LCD_CS_Clr();    // Ƭѡʹ��
    
    for(uint32_t i = 0; i < total_pixels; i++) {
        HAL_SPI_Transmit(&hspi1, &color_high, 1, HAL_MAX_DELAY);
        HAL_SPI_Transmit(&hspi1, &color_low, 1, HAL_MAX_DELAY);
    }
    LCD_CS_Set();    // Ƭѡ����
}

/**
 * @brief  ����LCD��ʾ����
 * @param  direction: ��ʾ����
 *         0: 0��    (Ĭ������)
 *         1: 90��   (˳ʱ��ת90��)
 *         2: 180��  (˳ʱ��ת180��)
 *         3: 270��  (˳ʱ��ת270��)
 * @note   ���÷����ͬʱ�����LCD�豸�Ŀ�Ⱥ͸߶Ȳ���
 * @retval None
 */
void LCD_SetDirection(uint8_t direction)
{
    // ���û�����LCD����
    lcddev.setxcmd = 0x2A;    // �е�ַ��������
    lcddev.setycmd = 0x2B;    // �е�ַ��������
    lcddev.wramcmd = 0x2C;    // дGRAM����
    
    switch(direction)
    {
        case 0:     // 0��
            lcddev.width = LCD_WIDTH;
            lcddev.height = LCD_HEIGHT;
            // BGR=1,MY=0,MX=0,MV=0: ������ʾ������ת������ת
            LCD_SendCmd(0x36);
            LCD_SendData((1<<3)|(0<<6)|(0<<7));
            break;
            
        case 1:     // 90��
            lcddev.width = LCD_HEIGHT;
            lcddev.height = LCD_WIDTH;
            // BGR=1,MY=0,MX=1,MV=1: X�������н���
            LCD_SendCmd(0x36);
            LCD_SendData((1<<3)|(0<<7)|(1<<6)|(1<<5));
            break;
            
        case 2:     // 180��
            lcddev.width = LCD_WIDTH;
            lcddev.height = LCD_HEIGHT;
            // BGR=1,MY=1,MX=1,MV=0: X��Y������
            LCD_SendCmd(0x36);
            LCD_SendData((1<<3)|(1<<6)|(1<<7));
            break;
            
        case 3:     // 270��
            lcddev.width = LCD_HEIGHT;
            lcddev.height = LCD_WIDTH;
            // BGR=1,MY=1,MX=0,MV=1: Y�������н���
            LCD_SendCmd(0x36);
            LCD_SendData((1<<3)|(1<<7)|(1<<5));
            break;
            
        default:
            break;
    }
}

void LCD_Init(void)
{
    // ��λLCD
    LCD_RESET();
    delay_ms(100);
 
    // ��ʼ��ʼ��LCD
    LCD_SendCmd(0xCF);
    LCD_SendData(0x00);
    LCD_SendData(0xD9);
    LCD_SendData(0X30);
    
    LCD_SendCmd(0xED);
    LCD_SendData(0x64);
    LCD_SendData(0x03);
    LCD_SendData(0X12);
    LCD_SendData(0X81);
    
    LCD_SendCmd(0xE8);
    LCD_SendData(0x85);
    LCD_SendData(0x10);
    LCD_SendData(0x7A);
    
    LCD_SendCmd(0xCB);
    LCD_SendData(0x39);
    LCD_SendData(0x2C);
    LCD_SendData(0x00);
    LCD_SendData(0x34);
    LCD_SendData(0x02);
    
    LCD_SendCmd(0xF7);
    LCD_SendData(0x20);
    
    LCD_SendCmd(0xEA);
    LCD_SendData(0x00);
    LCD_SendData(0x00);
    
    // Power Control
    LCD_SendCmd(0xC0);
    LCD_SendData(0x1B);
    
    LCD_SendCmd(0xC1);
    LCD_SendData(0x12);
    
    // VCM Control
    LCD_SendCmd(0xC5);
    LCD_SendData(0x08);
    LCD_SendData(0x26);
    
    LCD_SendCmd(0xC7);
    LCD_SendData(0XB7);
    
    // Memory Access Control
    LCD_SendCmd(0x36);
    LCD_SendData(0x08);
    
    LCD_SendCmd(0x3A);
    LCD_SendData(0x55);
    
    LCD_SendCmd(0xB1);
    LCD_SendData(0x00);
    LCD_SendData(0x1A);
    
    // Display Function Control
    LCD_SendCmd(0xB6);
    LCD_SendData(0x0A);
    LCD_SendData(0xA2);
    
    // 3Gamma Function
    LCD_SendCmd(0xF2);
    LCD_SendData(0x00);
    
    LCD_SendCmd(0x26);
    LCD_SendData(0x01);
    
    // Set Gamma
    LCD_SendCmd(0xE0);
    LCD_SendData(0x0F);
    LCD_SendData(0x1D);
    LCD_SendData(0x1A);
    LCD_SendData(0x0A);
    LCD_SendData(0x0D);
    LCD_SendData(0x07);
    LCD_SendData(0x49);
    LCD_SendData(0x66);
    LCD_SendData(0x3B);
    LCD_SendData(0x07);
    LCD_SendData(0x11);
    LCD_SendData(0x01);
    LCD_SendData(0x09);
    LCD_SendData(0x05);
    LCD_SendData(0x04);
    
    LCD_SendCmd(0XE1);
    LCD_SendData(0x00);
    LCD_SendData(0x18);
    LCD_SendData(0x1D);
    LCD_SendData(0x02);
    LCD_SendData(0x0F);
    LCD_SendData(0x04);
    LCD_SendData(0x36);
    LCD_SendData(0x13);
    LCD_SendData(0x4C);
    LCD_SendData(0x07);
    LCD_SendData(0x13);
    LCD_SendData(0x0F);
    LCD_SendData(0x2E);
    LCD_SendData(0x2F);
    LCD_SendData(0x05);
    
    // ������ʾ����
    LCD_SendCmd(0x2B);
    LCD_SendData(0x00);
    LCD_SendData(0x00);
    LCD_SendData(0x01);
    LCD_SendData(0x3F);
    
    LCD_SendCmd(0x2A);
    LCD_SendData(0x00);
    LCD_SendData(0x00);
    LCD_SendData(0x00);
    LCD_SendData(0xEF);
    
    // �˳�˯��ģʽ
    LCD_SendCmd(0x11);
    HAL_Delay(120);
    
    // ������ʾ
    LCD_SendCmd(0x29);
    
    // ������ʾ����
    LCD_SetDirection(USE_HORIZONTAL);
    
    // �������Ⲣ����
    LCD_BackLight_On();
    LCD_Clear(WHITE);
}


#define LCD_BUF_SIZE 8192  // DMAһ�����������
extern DMA_HandleTypeDef hdma_spi1_tx;  // DMA���
static uint8_t lcd_buf[LCD_BUF_SIZE] __attribute__((aligned(4)));
void LCD_FillBlock(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t *color)
{
#if USE_DMA_FillBlock
    uint16_t height, width;
    uint32_t byte_sum;        // ���ֽ���

    width = xend - xsta + 1;    // �������Ŀ��
    height = yend - ysta + 1;   // �������ĸ߶�

    byte_sum = width * height * 2; // ÿ�����������ֽ�

        LCD_SetWindow(xsta, ysta, xend, yend); // ����LCD���������
	
        LCD_DC_Set(); // д����
        LCD_CS_Clr();


        // ѭ����������ֱ���������������
        while (byte_sum > 0) {
                uint32_t bytes_to_send = (byte_sum > LCD_BUF_SIZE) ? LCD_BUF_SIZE : byte_sum;
            
                // ��� DMA ������
                for (uint32_t i = 0; i < bytes_to_send; i += 2) {
                        uint8_t high_byte = *color >> 8;        // ��8λ��ɫ
                        uint8_t low_byte =  *color & 0xFF;      // ��8λ��ɫ

                        lcd_buf[i] = high_byte;
                        lcd_buf[i + 1] = low_byte;
                        color ++;
                }

                // ��ʼ DMA ����
			// �ȴ�֮ǰ��DMA�������
			while(HAL_DMA_GetState(&hdma_spi1_tx) != HAL_DMA_STATE_READY);

			// ʹ��DMA��������
			HAL_SPI_Transmit_DMA(&hspi1, lcd_buf, bytes_to_send);

			// �ȴ��������
			while(HAL_DMA_GetState(&hdma_spi1_tx) != HAL_DMA_STATE_READY);

                // ����ʣ���ֽ���
                byte_sum -= bytes_to_send;
        }
        LCD_CS_Set();
#else
    uint16_t width = xend - xsta + 1;    // �������Ŀ��
    uint16_t height = yend - ysta + 1;   // �������ĸ߶�
    uint16_t color_buffer[width];
    // ������ʾ����
    LCD_SetWindow(xsta, ysta, xend, yend);
    
    // ׼��д����
    LCD_DC_Set();    // ����ģʽ
    LCD_CS_Clr();    // Ƭѡʹ��
    
    // �������
    for(uint16_t i = 0; i < height; i++)
    {
        for(uint16_t j = 0; j < width; j++)
        {
		color_buffer[j] = *color;
		color++;
        }
	   lcd_write_16bit_data_array(color_buffer, width);
    }
    
    LCD_CS_Set();    // Ƭѡ����
#endif
}



/*************************����ΪGUI**********************************************************************************/


/**
 * @brief  ��ָ��λ�û���
 * @param  startX: ���X����
 * @param  startY: ���Y����
 * @param  color:  �����ɫ(RGB565��ʽ)
 * @note   ʹ��SetWindow��������1x1����ʾ�����д����ɫ����
 * @retval None
 */
void LCD_DrawPoint(uint16_t startX, uint16_t startY, uint16_t color)
{
	// ����1x1����ʾ����
	LCD_SetWindow(startX, startY, startX, startY);
	LCD_WriteData_16Bit(color);
}


/*******************************************************************
 * @name       :����
 * @function   :Draw a line between two points
 * @parameters :	x1:the bebinning x coordinate of the line
				y1:the bebinning y coordinate of the line
				x2:the ending x coordinate of the line
				y2:the ending y coordinate of the line
 * @retvalue   :None
********************************************************************/
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 

	delta_x=x2-x1; //������������ 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //���õ������� 
	else if(delta_x==0)incx=0;//��ֱ�� 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//ˮƽ�� 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //ѡȡ�������������� 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//������� 
	{  
		LCD_DrawPoint(uRow,uCol, color);//���� 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
} 

/**
 * @brief  ��ָ��λ�û�ʵ��Բ
 * @param  centerX: Բ�ĵ�X����
 * @param  centerY: Բ�ĵ�Y����
 * @param  radius:  Բ�İ뾶
 * @param  color:   Բ����ɫ(RGB565��ʽ)
 * @retval None
 */
void LCD_DrawFilledCircle(uint16_t centerX, uint16_t centerY, uint16_t radius, uint16_t color)
{
    int16_t x = 0;
    int16_t y = radius;
    int16_t d = 1 - radius;
    int16_t deltaE = 3;
    int16_t deltaSE = -2 * radius + 5;

    // ����ʼ�İ˸���
    LCD_DrawLine(centerX - radius, centerY, centerX + radius, centerY, color);

    while (y > x)
    {
        if (d < 0) // ѡ�񶫵�
        {
            d += deltaE;
            deltaE += 2;
            deltaSE += 2;
        }
        else // ѡ���ϵ�
        {
            d += deltaSE;
            deltaE += 2;
            deltaSE += 4;
            y--;
        }
        x++;

        // ���ԳƵİ˸���
        LCD_DrawLine(centerX - x, centerY + y, centerX + x, centerY + y, color);
        LCD_DrawLine(centerX - x, centerY - y, centerX + x, centerY - y, color);
        LCD_DrawLine(centerX - y, centerY + x, centerX + y, centerY + x, color);
        LCD_DrawLine(centerX - y, centerY - x, centerX + y, centerY - x, color);
    }
}



