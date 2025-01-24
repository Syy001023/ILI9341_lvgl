#include "24cxx.h"

/**
 * @brief  ��ʼ��AT24CXX
 * @param  None
 * @retval None
 */
void AT24CXX_Init(void)
{
    IIC_Init();
}

/**
 * @brief  ��AT24CXXָ����ַ����һ������
 * @param  ReadAddr: ��ʼ�����ĵ�ַ  
 * @retval ����������
 */
uint8_t AT24CXX_ReadOneByte(uint16_t ReadAddr)
{				  
    uint8_t temp = 0;		  	    																 
    IIC_Start();  
    
    if(EE_TYPE > AT24C16)
    {
        IIC_Send_Byte(AT24CXX_ADDR);     // ����д����
        IIC_Wait_Ack();
        IIC_Send_Byte(ReadAddr >> 8);     // ���͸ߵ�ַ
        IIC_Wait_Ack();		 
    }
    else
    {
        IIC_Send_Byte(AT24CXX_ADDR + ((ReadAddr/256) << 1));   // ����������ַ0XA0,д���� 	 
    }

    IIC_Wait_Ack(); 
    IIC_Send_Byte(ReadAddr % 256);   // ���͵͵�ַ
    IIC_Wait_Ack();	    
    IIC_Start();  	 	   
    IIC_Send_Byte(AT24CXX_ADDR | 0x01);           // �������ģʽ			   
    IIC_Wait_Ack();	 
    temp = IIC_Read_Byte(0);		   
    IIC_Stop();
    
    return temp;
}

/**
 * @brief  ��AT24CXXָ����ַд��һ������
 * @param  WriteAddr: д�����ݵ�Ŀ�ĵ�ַ    
 * @param  DataToWrite: Ҫд�������
 * @retval None
 */
void AT24CXX_WriteOneByte(uint16_t WriteAddr, uint8_t DataToWrite)
{				   	  	    																 
    IIC_Start();  
    
    if(EE_TYPE > AT24C16)
    {
        IIC_Send_Byte(AT24CXX_ADDR);     // ����д����
        IIC_Wait_Ack();
        IIC_Send_Byte(WriteAddr >> 8);    // ���͸ߵ�ַ	  
        IIC_Wait_Ack();		 
    }
    else
    {
        IIC_Send_Byte(AT24CXX_ADDR + ((WriteAddr/256) << 1));   // ����������ַ0XA0,д����
    }
    
    IIC_Wait_Ack();	   
    IIC_Send_Byte(WriteAddr % 256);   // ���͵͵�ַ
    IIC_Wait_Ack(); 	 										  		   
    IIC_Send_Byte(DataToWrite);     // �����ֽ�							   
    IIC_Wait_Ack();  		    	   
    IIC_Stop();
    
    HAL_Delay(10);    // ע�⣺����Ҫ�ȴ�д�����
}

/**
 * @brief  ��AT24CXX�����ָ����ַ��ʼд�볤��ΪLen������
 * @param  WriteAddr: ��ʼд��ĵ�ַ  
 * @param  DataToWrite: ���������׵�ַ
 * @param  Len: Ҫд�����ݵĳ���2,4
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
 * @brief  ��AT24CXX�����ָ����ַ��ʼ��������ΪLen������
 * @param  ReadAddr: ��ʼ�����ĵ�ַ 
 * @param  Len: Ҫ�������ݵĳ���2,4
 * @retval ��ȡ��������
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
 * @brief  ���AT24CXX�Ƿ�����
 * @param  None
 * @retval 0: ���ɹ�
 *         1: ���ʧ��
 */
uint8_t AT24CXX_Check(void)
{
    uint8_t temp;
    temp = AT24CXX_ReadOneByte(255);     // ����ÿ�ο�����дAT24CXX			   
    if(temp == 0x55)
        return 0;		   
    else    // �ų���һ�γ�ʼ�������
    {
        AT24CXX_WriteOneByte(255, 0x55);
        temp = AT24CXX_ReadOneByte(255);	  
        if(temp == 0x55)
            return 0;
    }
    return 1;											  
}

/**
 * @brief  ��AT24CXX�����ָ����ַ��ʼ����ָ������������
 * @param  ReadAddr: ��ʼ�����ĵ�ַ 
 * @param  pBuffer: ���������׵�ַ
 * @param  NumToRead: Ҫ�������ݵĸ���
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
 * @brief  ��AT24CXX�����ָ����ַ��ʼд��ָ������������
 * @param  WriteAddr: ��ʼд��ĵ�ַ 
 * @param  pBuffer: ���������׵�ַ
 * @param  NumToWrite: Ҫд�����ݵĸ���
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
