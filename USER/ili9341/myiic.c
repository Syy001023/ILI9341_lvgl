#include "myiic.h"
#include "delay.h"

/**
 * @brief  ��ʼ��IIC
 * @param  None
 * @retval None
 */
void IIC_Init(void)
{
//    GPIO_InitTypeDef GPIO_InitStruct = {0};

//    // ʹ��GPIOBʱ��
//    __HAL_RCC_GPIOB_CLK_ENABLE();

//    // ����SCL��SDA����
//    GPIO_InitStruct.Pin = IIC_SCL_Pin | IIC_SDA_Pin;
//    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
//    GPIO_InitStruct.Pull = GPIO_PULLUP;
//    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // ����״̬��������
    IIC_SCL_SET();
    IIC_SDA_SET();
}

/**
 * @brief  ����IIC��ʼ�ź�
 * @param  None
 * @retval None
 */
void IIC_Start(void)
{
    SDA_OUT();
    IIC_SDA_SET();
    IIC_SCL_SET();
    delay_us(4);
    IIC_SDA_CLR();     // START: ��SCLΪ��ʱ��SDA�Ӹ߱��
    delay_us(4);
    IIC_SCL_CLR();     // ǯסI2C���ߣ�׼�����ͻ��������
}

/**
 * @brief  ����IICֹͣ�ź�
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
    IIC_SDA_SET();     // STOP: ��SCLΪ��ʱ��SDA�ӵͱ��
    delay_us(4);
}

/**
 * @brief  �ȴ�Ӧ���źŵ���
 * @param  None
 * @retval 1: ����Ӧ��ʧ��
 *         0: ����Ӧ��ɹ�
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
 * @brief  ����ACKӦ��
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
 * @brief  ������ACKӦ��
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
 * @brief  IIC����һ���ֽ�
 * @param  txd: Ҫ���͵��ֽ�
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
 * @brief  IIC��ȡһ���ֽ�
 * @param  ack: 1������ACK��0������NACK
 * @retval ��ȡ�����ֽ�
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
 * @brief  д��һ���ֽڵ�ָ���豸��ָ����ַ
 * @param  daddr: �豸��ַ
 * @param  addr: �Ĵ�����ַ
 * @param  data: Ҫд�������
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
 * @brief  ��ָ���豸��ָ����ַ��ȡһ���ֽ�
 * @param  daddr: �豸��ַ
 * @param  addr: �Ĵ�����ַ
 * @retval ��ȡ��������
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
