#include "ili9341.h"
#include "spi.h"
#include "dma.h"

_lcd_dev lcddev;

/**
 * @brief  发送命令到ST7789显示屏
 * @param  cmd: 要发送的命令字节
 * @note   发送命令时，DC引脚需要置低，CS引脚先置低后置高
 * @retval None
 */
void LCD_SendCmd(uint8_t cmd)
{
    LCD_DC_Clr();    // 命令模式
    LCD_CS_Clr();    // 片选使能
    
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    
    LCD_CS_Set();    // 片选禁用
}

/**
 * @brief  发送数据到ST7789显示屏
 * @param  data: 要发送的数据字节
 * @note   发送数据时，DC引脚需要置高，CS引脚先置低后置高
 * @retval None
 */
void LCD_SendData(uint8_t data)
{
    LCD_DC_Set();    // 数据模式
    LCD_CS_Clr();    // 片选使能
    
    HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
    
    LCD_CS_Set();    // 片选禁用
}

/**
 * @brief  向LCD写入寄存器和数据
 * @param  reg: 寄存器地址
 * @param  data: 要写入的数据
 * @retval None
 */
void LCD_WriteReg(uint8_t reg, uint16_t data)
{
    LCD_SendCmd(reg);
    LCD_SendData(data);
}

/**
 * @brief  准备向LCD GRAM写入数据
 * @note   在写入像素数据之前需要调用此函数
 * @param  None
 * @retval None
 */
void LCD_WriteRAM_Prepare(void)
{
    LCD_SendCmd(lcddev.wramcmd);
}

/**
 * @brief  向LCD写入16位数据
 * @param  data: 要写入的16位数据
 * @retval None
 */
void LCD_WriteData_16Bit(uint16_t data)
{
    LCD_DC_Set();    // 数据模式
    LCD_CS_Clr();    // 片选使能
    
    // 先发送高8位，再发送低8位
    HAL_SPI_Transmit(&hspi1, (uint8_t*)&data + 1, 1, HAL_MAX_DELAY);
    HAL_SPI_Transmit(&hspi1, (uint8_t*)&data, 1, HAL_MAX_DELAY);
    
    LCD_CS_Set();    // 片选禁用
}

void lcd_write_16bit_data_array(const uint16_t *data, uint32_t len)
{
    uint16_t i;
    LCD_CS_Clr();
    LCD_DC_Set(); 
    for (i = 0; i < len; i++)
    {
        uint8_t high_byte = data[i] >> 8;        // 获取高8位
        uint8_t low_byte = data[i] & 0xFF;       // 获取低8位
        
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
 * @brief  设置LCD显示窗口
 * @details 设置显示窗口后可以连续发送颜色数据，无需重复设置坐标
 * @param  startX: 窗口起点x轴坐标
 * @param  startY: 窗口起点y轴坐标
 * @param  width:  窗口宽度
 * @param  height: 窗口高度
 * @note   设置显示区域后会自动开启写入模式(RAMWR)
 * @retval None
 */
void LCD_SetWindow(uint16_t startX, uint16_t startY, uint16_t endX, uint16_t endY)
{
    // 设置列地址范围
    LCD_SendCmd(0x2A);        // Column Address Set
    LCD_SendData(startX >> 8);     // 起始列地址高8位
    LCD_SendData(startX & 0xFF);   // 起始列地址低8位
    LCD_SendData(endX >> 8);       // 结束列地址高8位
    LCD_SendData(endX & 0xFF);     // 结束列地址低8位

    // 设置行地址范围
    LCD_SendCmd(0x2B);        // Row Address Set
    LCD_SendData(startY >> 8);     // 起始行地址高8位
    LCD_SendData(startY & 0xFF);   // 起始行地址低8位
    LCD_SendData(endY >> 8);       // 结束行地址高8位
    LCD_SendData(endY & 0xFF);     // 结束行地址低8位
    
    // 开始写入显存
    LCD_SendCmd(0x2C);        // Memory Write
}

/**
 * @brief  设置LCD光标位置
 * @param  x: x坐标
 * @param  y: y坐标
 * @retval None
 */
void LCD_SetCursor(uint16_t x, uint16_t y)
{
    LCD_SetWindow(x, y, x, y);
}

/**
 * @brief  使用指定颜色填充整个屏幕
 * @param  color: 16位RGB565格式的颜色值
 * @note   使用连续写入模式快速填充
 * @retval None
 */
void LCD_Clear(uint16_t color)
{
    uint32_t total_pixels = lcddev.width * lcddev.height;
    uint8_t color_high = color >> 8;
    uint8_t color_low = color & 0xFF;
    
    // 设置全屏显示窗口
    LCD_SetWindow(0, 0, lcddev.width, lcddev.height);
    
    // 连续发送颜色数据
    LCD_DC_Set();    // 数据模式
    LCD_CS_Clr();    // 片选使能
    
    for(uint32_t i = 0; i < total_pixels; i++) {
        HAL_SPI_Transmit(&hspi1, &color_high, 1, HAL_MAX_DELAY);
        HAL_SPI_Transmit(&hspi1, &color_low, 1, HAL_MAX_DELAY);
    }
    LCD_CS_Set();    // 片选禁用
}

/**
 * @brief  设置LCD显示方向
 * @param  direction: 显示方向
 *         0: 0度    (默认竖屏)
 *         1: 90度   (顺时针转90度)
 *         2: 180度  (顺时针转180度)
 *         3: 270度  (顺时针转270度)
 * @note   设置方向的同时会更新LCD设备的宽度和高度参数
 * @retval None
 */
void LCD_SetDirection(uint8_t direction)
{
    // 设置基本的LCD命令
    lcddev.setxcmd = 0x2A;    // 列地址设置命令
    lcddev.setycmd = 0x2B;    // 行地址设置命令
    lcddev.wramcmd = 0x2C;    // 写GRAM命令
    
    switch(direction)
    {
        case 0:     // 0度
            lcddev.width = LCD_WIDTH;
            lcddev.height = LCD_HEIGHT;
            // BGR=1,MY=0,MX=0,MV=0: 正常显示，不翻转，不旋转
            LCD_SendCmd(0x36);
            LCD_SendData((1<<3)|(0<<6)|(0<<7));
            break;
            
        case 1:     // 90度
            lcddev.width = LCD_HEIGHT;
            lcddev.height = LCD_WIDTH;
            // BGR=1,MY=0,MX=1,MV=1: X镜像，行列交换
            LCD_SendCmd(0x36);
            LCD_SendData((1<<3)|(0<<7)|(1<<6)|(1<<5));
            break;
            
        case 2:     // 180度
            lcddev.width = LCD_WIDTH;
            lcddev.height = LCD_HEIGHT;
            // BGR=1,MY=1,MX=1,MV=0: X和Y都镜像
            LCD_SendCmd(0x36);
            LCD_SendData((1<<3)|(1<<6)|(1<<7));
            break;
            
        case 3:     // 270度
            lcddev.width = LCD_HEIGHT;
            lcddev.height = LCD_WIDTH;
            // BGR=1,MY=1,MX=0,MV=1: Y镜像，行列交换
            LCD_SendCmd(0x36);
            LCD_SendData((1<<3)|(1<<7)|(1<<5));
            break;
            
        default:
            break;
    }
}

void LCD_Init(void)
{
    // 复位LCD
    LCD_RESET();
    delay_ms(100);
 
    // 开始初始化LCD
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
    
    // 设置显示区域
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
    
    // 退出睡眠模式
    LCD_SendCmd(0x11);
    HAL_Delay(120);
    
    // 开启显示
    LCD_SendCmd(0x29);
    
    // 设置显示方向
    LCD_SetDirection(USE_HORIZONTAL);
    
    // 开启背光并清屏
    LCD_BackLight_On();
    LCD_Clear(WHITE);
}


#define LCD_BUF_SIZE 8192  // DMA一次最大传输数量
extern DMA_HandleTypeDef hdma_spi1_tx;  // DMA句柄
static uint8_t lcd_buf[LCD_BUF_SIZE] __attribute__((aligned(4)));
void LCD_FillBlock(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t *color)
{
#if USE_DMA_FillBlock
    uint16_t height, width;
    uint32_t byte_sum;        // 总字节数

    width = xend - xsta + 1;    // 计算填充的宽度
    height = yend - ysta + 1;   // 计算填充的高度

    byte_sum = width * height * 2; // 每个像素两个字节

        LCD_SetWindow(xsta, ysta, xend, yend); // 设置LCD的填充区域
	
        LCD_DC_Set(); // 写数据
        LCD_CS_Clr();


        // 循环发送数据直到填充完整个区域
        while (byte_sum > 0) {
                uint32_t bytes_to_send = (byte_sum > LCD_BUF_SIZE) ? LCD_BUF_SIZE : byte_sum;
            
                // 填充 DMA 缓冲区
                for (uint32_t i = 0; i < bytes_to_send; i += 2) {
                        uint8_t high_byte = *color >> 8;        // 高8位颜色
                        uint8_t low_byte =  *color & 0xFF;      // 低8位颜色

                        lcd_buf[i] = high_byte;
                        lcd_buf[i + 1] = low_byte;
                        color ++;
                }

                // 开始 DMA 传输
			// 等待之前的DMA传输完成
			while(HAL_DMA_GetState(&hdma_spi1_tx) != HAL_DMA_STATE_READY);

			// 使用DMA发送数据
			HAL_SPI_Transmit_DMA(&hspi1, lcd_buf, bytes_to_send);

			// 等待传输完成
			while(HAL_DMA_GetState(&hdma_spi1_tx) != HAL_DMA_STATE_READY);

                // 更新剩余字节数
                byte_sum -= bytes_to_send;
        }
        LCD_CS_Set();
#else
    uint16_t width = xend - xsta + 1;    // 计算填充的宽度
    uint16_t height = yend - ysta + 1;   // 计算填充的高度
    uint16_t color_buffer[width];
    // 设置显示窗口
    LCD_SetWindow(xsta, ysta, xend, yend);
    
    // 准备写数据
    LCD_DC_Set();    // 数据模式
    LCD_CS_Clr();    // 片选使能
    
    // 按行填充
    for(uint16_t i = 0; i < height; i++)
    {
        for(uint16_t j = 0; j < width; j++)
        {
		color_buffer[j] = *color;
		color++;
        }
	   lcd_write_16bit_data_array(color_buffer, width);
    }
    
    LCD_CS_Set();    // 片选禁用
#endif
}



/*************************以下为GUI**********************************************************************************/


/**
 * @brief  在指定位置画点
 * @param  startX: 点的X坐标
 * @param  startY: 点的Y坐标
 * @param  color:  点的颜色(RGB565格式)
 * @note   使用SetWindow函数设置1x1的显示区域后写入颜色数据
 * @retval None
 */
void LCD_DrawPoint(uint16_t startX, uint16_t startY, uint16_t color)
{
	// 设置1x1的显示窗口
	LCD_SetWindow(startX, startY, startX, startY);
	LCD_WriteData_16Bit(color);
}


/*******************************************************************
 * @name       :画线
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

	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //设置单步方向 
	else if(delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//水平线 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//画线输出 
	{  
		LCD_DrawPoint(uRow,uCol, color);//画点 
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
 * @brief  在指定位置画实心圆
 * @param  centerX: 圆心的X坐标
 * @param  centerY: 圆心的Y坐标
 * @param  radius:  圆的半径
 * @param  color:   圆的颜色(RGB565格式)
 * @retval None
 */
void LCD_DrawFilledCircle(uint16_t centerX, uint16_t centerY, uint16_t radius, uint16_t color)
{
    int16_t x = 0;
    int16_t y = radius;
    int16_t d = 1 - radius;
    int16_t deltaE = 3;
    int16_t deltaSE = -2 * radius + 5;

    // 画初始的八个点
    LCD_DrawLine(centerX - radius, centerY, centerX + radius, centerY, color);

    while (y > x)
    {
        if (d < 0) // 选择东点
        {
            d += deltaE;
            deltaE += 2;
            deltaSE += 2;
        }
        else // 选择东南点
        {
            d += deltaSE;
            deltaE += 2;
            deltaSE += 4;
            y--;
        }
        x++;

        // 画对称的八个点
        LCD_DrawLine(centerX - x, centerY + y, centerX + x, centerY + y, color);
        LCD_DrawLine(centerX - x, centerY - y, centerX + x, centerY - y, color);
        LCD_DrawLine(centerX - y, centerY + x, centerX + y, centerY + x, color);
        LCD_DrawLine(centerX - y, centerY - x, centerX + y, centerY - x, color);
    }
}



