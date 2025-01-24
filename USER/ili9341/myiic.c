#include "myiic.h"
#include "delay.h"

/**
 * @brief  初始化IIC
 * @param  None
 * @retval None
 */
void IIC_Init(void)
{
//    GPIO_InitTypeDef GPIO_InitStruct = {0};

//    // 使能GPIOB时钟
//    __HAL_RCC_GPIOB_CLK_ENABLE();

//    // 配置SCL和SDA引脚
//    GPIO_InitStruct.Pin = IIC_SCL_Pin | IIC_SDA_Pin;
//    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
//    GPIO_InitStruct.Pull = GPIO_PULLUP;
//    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // 空闲状态，都拉高
    IIC_SCL_SET();
    IIC_SDA_SET();
}

/**
 * @brief  产生IIC起始信号
 * @param  None
 * @retval None
 */
void IIC_Start(void)
{
    SDA_OUT();
    IIC_SDA_SET();
    IIC_SCL_SET();
    delay_us(4);
    IIC_SDA_CLR();     // START: 当SCL为高时，SDA从高变低
    delay_us(4);
    IIC_SCL_CLR();     // 钳住I2C总线，准备发送或接收数据
}

/**
 * @brief  产生IIC停止信号
 * @param  None
 * @retval None
 */
void IIC_Stop(void)
{
    SDA_OUT();
    IIC_SCL_CLR();
    IIC_SDA_CLR();
    delay_us(4);
    IIC_SCL_SET();
    IIC_SDA_SET();     // STOP: 当SCL为高时，SDA从低变高
    delay_us(4);
}

/**
 * @brief  等待应答信号到来
 * @param  None
 * @retval 1: 接收应答失败
 *         0: 接收应答成功
 */
uint8_t IIC_Wait_Ack(void)
{
    uint8_t ucErrTime = 0;

    SDA_IN();
    IIC_SDA_SET();
    delay_us(1);
    IIC_SCL_SET();
    delay_us(1);

    while(IIC_SDA_READ())
    {
        ucErrTime++;
        if(ucErrTime > 250)
        {
            IIC_Stop();
            return 1;
        }
    }
    IIC_SCL_CLR();
    return 0;
}

/**
 * @brief  产生ACK应答
 * @param  None
 * @retval None
 */
void IIC_Ack(void)
{
    IIC_SCL_CLR();
    SDA_OUT();
    IIC_SDA_CLR();
    delay_us(2);
    IIC_SCL_SET();
    delay_us(2);
    IIC_SCL_CLR();
}

/**
 * @brief  不产生ACK应答
 * @param  None
 * @retval None
 */
void IIC_NAck(void)
{
    IIC_SCL_CLR();
    SDA_OUT();
    IIC_SDA_SET();
    delay_us(2);
    IIC_SCL_SET();
    delay_us(2);
    IIC_SCL_CLR();
}

/**
 * @brief  IIC发送一个字节
 * @param  txd: 要发送的字节
 * @retval None
 */
void IIC_Send_Byte(uint8_t txd)
{
    uint8_t t;
    SDA_OUT();
    IIC_SCL_CLR();

    for(t=0; t<8; t++)
    {
        if((txd&0x80)>>7)
            IIC_SDA_SET();
        else
            IIC_SDA_CLR();
        txd<<=1;
        delay_us(2);
        IIC_SCL_SET();
        delay_us(2);
        IIC_SCL_CLR();
        delay_us(2);
    }
}

/**
 * @brief  IIC读取一个字节
 * @param  ack: 1，发送ACK；0，发送NACK
 * @retval 读取到的字节
 */
uint8_t IIC_Read_Byte(uint8_t ack)
{
    uint8_t i, receive=0;
    SDA_IN();
    for(i=0; i<8; i++)
    {
        IIC_SCL_CLR();
        delay_us(2);
        IIC_SCL_SET();
        receive<<=1;
        if(IIC_SDA_READ())
            receive++;
        delay_us(1);
    }
    if (!ack)
        IIC_NAck();
    else
        IIC_Ack();
    return receive;
}

/**
 * @brief  写入一个字节到指定设备的指定地址
 * @param  daddr: 设备地址
 * @param  addr: 寄存器地址
 * @param  data: 要写入的数据
 * @retval None
 */
void IIC_Write_One_Byte(uint8_t daddr, uint8_t addr, uint8_t data)
{
    IIC_Start();
    IIC_Send_Byte(daddr);
    IIC_Wait_Ack();
    IIC_Send_Byte(addr);
    IIC_Wait_Ack();
    IIC_Send_Byte(data);
    IIC_Wait_Ack();
    IIC_Stop();
}

/**
 * @brief  从指定设备的指定地址读取一个字节
 * @param  daddr: 设备地址
 * @param  addr: 寄存器地址
 * @retval 读取到的数据
 */
uint8_t IIC_Read_One_Byte(uint8_t daddr, uint8_t addr)
{
    uint8_t temp = 0;

    IIC_Start();
    IIC_Send_Byte(daddr);
    IIC_Wait_Ack();
    IIC_Send_Byte(addr);
    IIC_Wait_Ack();
    IIC_Start();
    IIC_Send_Byte(daddr|0x01);
    IIC_Wait_Ack();
    temp = IIC_Read_Byte(0);
    IIC_Stop();

    return temp;
}
