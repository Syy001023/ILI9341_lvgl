#include "24cxx.h"

/**
 * @brief  初始化AT24CXX
 * @param  None
 * @retval None
 */
void AT24CXX_Init(void)
{
    IIC_Init();
}

/**
 * @brief  在AT24CXX指定地址读出一个数据
 * @param  ReadAddr: 开始读数的地址  
 * @retval 读到的数据
 */
uint8_t AT24CXX_ReadOneByte(uint16_t ReadAddr)
{				  
    uint8_t temp = 0;		  	    																 
    IIC_Start();  
    
    if(EE_TYPE > AT24C16)
    {
        IIC_Send_Byte(AT24CXX_ADDR);     // 发送写命令
        IIC_Wait_Ack();
        IIC_Send_Byte(ReadAddr >> 8);     // 发送高地址
        IIC_Wait_Ack();		 
    }
    else
    {
        IIC_Send_Byte(AT24CXX_ADDR + ((ReadAddr/256) << 1));   // 发送器件地址0XA0,写数据 	 
    }

    IIC_Wait_Ack(); 
    IIC_Send_Byte(ReadAddr % 256);   // 发送低地址
    IIC_Wait_Ack();	    
    IIC_Start();  	 	   
    IIC_Send_Byte(AT24CXX_ADDR | 0x01);           // 进入接收模式			   
    IIC_Wait_Ack();	 
    temp = IIC_Read_Byte(0);		   
    IIC_Stop();
    
    return temp;
}

/**
 * @brief  在AT24CXX指定地址写入一个数据
 * @param  WriteAddr: 写入数据的目的地址    
 * @param  DataToWrite: 要写入的数据
 * @retval None
 */
void AT24CXX_WriteOneByte(uint16_t WriteAddr, uint8_t DataToWrite)
{				   	  	    																 
    IIC_Start();  
    
    if(EE_TYPE > AT24C16)
    {
        IIC_Send_Byte(AT24CXX_ADDR);     // 发送写命令
        IIC_Wait_Ack();
        IIC_Send_Byte(WriteAddr >> 8);    // 发送高地址	  
        IIC_Wait_Ack();		 
    }
    else
    {
        IIC_Send_Byte(AT24CXX_ADDR + ((WriteAddr/256) << 1));   // 发送器件地址0XA0,写数据
    }
    
    IIC_Wait_Ack();	   
    IIC_Send_Byte(WriteAddr % 256);   // 发送低地址
    IIC_Wait_Ack(); 	 										  		   
    IIC_Send_Byte(DataToWrite);     // 发送字节							   
    IIC_Wait_Ack();  		    	   
    IIC_Stop();
    
    HAL_Delay(10);    // 注意：这里要等待写入完成
}

/**
 * @brief  在AT24CXX里面的指定地址开始写入长度为Len的数据
 * @param  WriteAddr: 开始写入的地址  
 * @param  DataToWrite: 数据数组首地址
 * @param  Len: 要写入数据的长度2,4
 * @retval None
 */
void AT24CXX_WriteLenByte(uint16_t WriteAddr, uint32_t DataToWrite, uint8_t Len)
{  	
    for(uint8_t t = 0; t < Len; t++)
    {
        AT24CXX_WriteOneByte(WriteAddr + t, (DataToWrite >> (8*t)) & 0xFF);
    }												    
}

/**
 * @brief  在AT24CXX里面的指定地址开始读出长度为Len的数据
 * @param  ReadAddr: 开始读出的地址 
 * @param  Len: 要读出数据的长度2,4
 * @retval 读取到的数据
 */
uint32_t AT24CXX_ReadLenByte(uint16_t ReadAddr, uint8_t Len)
{  	
    uint32_t temp = 0;
    for(uint8_t t = 0; t < Len; t++)
    {
        temp <<= 8;
        temp += AT24CXX_ReadOneByte(ReadAddr + Len - t - 1); 	 				   
    }
    return temp;												    
}

/**
 * @brief  检查AT24CXX是否正常
 * @param  None
 * @retval 0: 检测成功
 *         1: 检测失败
 */
uint8_t AT24CXX_Check(void)
{
    uint8_t temp;
    temp = AT24CXX_ReadOneByte(255);     // 避免每次开机都写AT24CXX			   
    if(temp == 0x55)
        return 0;		   
    else    // 排除第一次初始化的情况
    {
        AT24CXX_WriteOneByte(255, 0x55);
        temp = AT24CXX_ReadOneByte(255);	  
        if(temp == 0x55)
            return 0;
    }
    return 1;											  
}

/**
 * @brief  在AT24CXX里面的指定地址开始读出指定个数的数据
 * @param  ReadAddr: 开始读出的地址 
 * @param  pBuffer: 数据数组首地址
 * @param  NumToRead: 要读出数据的个数
 * @retval None
 */
void AT24CXX_Read(uint16_t ReadAddr, uint8_t *pBuffer, uint16_t NumToRead)
{
    while(NumToRead)
    {
        *pBuffer++ = AT24CXX_ReadOneByte(ReadAddr++);	
        NumToRead--;
    }
}  

/**
 * @brief  在AT24CXX里面的指定地址开始写入指定个数的数据
 * @param  WriteAddr: 开始写入的地址 
 * @param  pBuffer: 数据数组首地址
 * @param  NumToWrite: 要写入数据的个数
 * @retval None
 */
void AT24CXX_Write(uint16_t WriteAddr, uint8_t *pBuffer, uint16_t NumToWrite)
{
    while(NumToWrite--)
    {
        AT24CXX_WriteOneByte(WriteAddr, *pBuffer);
        WriteAddr++;
        pBuffer++;
    }
}
