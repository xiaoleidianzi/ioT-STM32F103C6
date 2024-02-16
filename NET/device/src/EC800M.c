/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	EC800M.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2017-05-08
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		EC800M�ļ�����
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��ͷ�ļ�
#include "stm32f10x.h"

//�����豸����
#include "EC800M.h"

//Ӳ������
#include "delay.h"
#include "usart.h"

//C��
#include <string.h>
#include <stdio.h>

//���������ļ�
#include "Config.h"


unsigned char EC800M_buf[512];
unsigned short EC800M_cnt = 0, EC800M_cntPre = 0;


//==========================================================
//	�������ƣ�	EC800M_Clear
//
//	�������ܣ�	��ջ���
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void EC800M_Clear(void)
{

	memset(EC800M_buf, 0, sizeof(EC800M_buf));
	EC800M_cnt = 0;

}

//==========================================================
//	�������ƣ�	EC800M_WaitRecive
//
//	�������ܣ�	�ȴ��������
//
//	��ڲ�����	��
//
//	���ز�����	REV_OK-�������		REV_WAIT-���ճ�ʱδ���
//
//	˵����		ѭ�����ü���Ƿ�������
//==========================================================
_Bool EC800M_WaitRecive(void)
{

	if(EC800M_cnt == 0) 							//������ռ���Ϊ0 ��˵��û�д��ڽ��������У�����ֱ����������������
		return REV_WAIT;
		
	if(EC800M_cnt == EC800M_cntPre)				//�����һ�ε�ֵ�������ͬ����˵���������
	{
		EC800M_cnt = 0;							//��0���ռ���
			
		return REV_OK;								//���ؽ�����ɱ�־
	}
		
	EC800M_cntPre = EC800M_cnt;					//��Ϊ��ͬ
	
	return REV_WAIT;								//���ؽ���δ��ɱ�־

}

//==========================================================
//	�������ƣ�	EC800M_SendCmd
//
//	�������ܣ�	��������
//
//	��ڲ�����	cmd������
//				res����Ҫ���ķ���ָ��
//
//	���ز�����	0-�ɹ�	1-ʧ��
//
//	˵����		
//==========================================================
_Bool EC800M_SendCmd(char *cmd, char *res)
{
	
	unsigned char timeOut = 200;

	Usart_SendString(USART2, cmd, strlen((const char *)cmd));

	while(timeOut--)
	{
		if(EC800M_WaitRecive() == REV_OK)							//����յ�����
		{
			if(strstr((const char *)EC800M_buf, res) != NULL)		//����������ؼ���
			{
				EC800M_Clear();									//��ջ���
				
				return 0;
			}
		}
		
		DelayXms(10);
	}
	
	return 1;

}

//==========================================================
//	�������ƣ�	EC800M_SendData
//
//	�������ܣ�	��������
//
//	��ڲ�����	data������
//				len������
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void EC800M_SendData(unsigned char *data, unsigned short len)
{

	char cmdBuf[32];
	
	EC800M_Clear();								//��ս��ջ���
	sprintf(cmdBuf, "AT+CIPSEND=%d\r\n", len);		//��������
	if(!EC800M_SendCmd(cmdBuf, ">"))				//�յ���>��ʱ���Է�������
	{
		Usart_SendString(USART2, data, len);		//�����豸������������
	}

}

//==========================================================
//	�������ƣ�	EC800M_GetIPD
//
//	�������ܣ�	��ȡƽ̨���ص�����
//
//	��ڲ�����	�ȴ���ʱ��(����10ms)
//
//	���ز�����	ƽ̨���ص�ԭʼ����
//
//	˵����		��ͬ�����豸���صĸ�ʽ��ͬ����Ҫȥ����
//				��EC800M�ķ��ظ�ʽΪ	"+IPD,x:yyy"	x�������ݳ��ȣ�yyy����������
//==========================================================
unsigned char *EC800M_GetIPD(unsigned short timeOut)
{

	char *ptrIPD = NULL;
	
	do
	{
		if(EC800M_WaitRecive() == REV_OK)								//����������
		{
			ptrIPD = strstr((char *)EC800M_buf, "IPD,");				//������IPD��ͷ
			if(ptrIPD == NULL)											//���û�ҵ���������IPDͷ���ӳ٣�������Ҫ�ȴ�һ�ᣬ�����ᳬ���趨��ʱ��
			{
				//UsartPrintf(USART_DEBUG, "\"IPD\" not found\r\n");
			}
			else
			{
				ptrIPD = strchr(ptrIPD, ':');							//�ҵ�':'
				if(ptrIPD != NULL)
				{
					ptrIPD++;
					return (unsigned char *)(ptrIPD);
				}
				else
					return NULL;
				
			}
		}
		
		DelayXms(5);													//��ʱ�ȴ�
	} while(timeOut--);
	
	return NULL;														//��ʱ��δ�ҵ������ؿ�ָ��

}

//==========================================================
//	�������ƣ�	EC800M_Init
//
//	�������ܣ�	��ʼ��EC800M
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void EC800M_Init(void)
{
	
	//GPIO_InitTypeDef GPIO_Initure;
	
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_WriteBit(GPIOB, GPIO_Pin_8, Bit_RESET);
	DelayXms(250);
	GPIO_WriteBit(GPIOB, GPIO_Pin_8, Bit_SET);
	DelayXms(500);
	
	EC800M_Clear();
		
	UsartPrintf(USART_DEBUG, "1.AT+QMTCFG=\"version\",0,4\r\n");
	while(EC800M_SendCmd("AT+QMTCFG=\"version\",0,4\r\n", "OK"))
		DelayXms(500);
	
	UsartPrintf(USART_DEBUG, "2.AT+QMTCFG=\"recv/mode\",0,0,1\r\n");
	while(EC800M_SendCmd("AT+QMTCFG=\"recv/mode\",0,0,1\r\n", "OK"))
		DelayXms(500);
	
	UsartPrintf(USART_DEBUG, "3.AT+QMTCFG=\"onenet\",0,\"470285\",\"TmZ5csCJjqGS/Q1QaWwtY0cB+mWY3BEZQz1Ml52NiuQ=\"\r\n");
	while(EC800M_SendCmd(EC800M_ONENET_INFO, "OK"))
		DelayXms(500);
	
	UsartPrintf(USART_DEBUG, "4.AT+QMTOPEN=0,\"mqtts.heclouds.com\",1883\r\n");
	while(EC800M_SendCmd("AT+QMTOPEN=0,\"mqtts.heclouds.com\",1883\r\n", "OK"))
		DelayXms(500);
	
	UsartPrintf(USART_DEBUG, "5.AT+QMTCONN=0,\"my_first_device\"\r\n");
	while(EC800M_SendCmd(EC800M_DEVICE_INFO,"OK")) //�豸ע��
		DelayXms(500);
	
	UsartPrintf(USART_DEBUG, "6. EC800M Init OK\r\n");

}

//==========================================================
//	�������ƣ�	USART2_IRQHandler
//
//	�������ܣ�	����2�շ��ж�
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void EC800M_USART2_IRQHandler(void)
{

	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) //�����ж�
	{
		if(EC800M_cnt >= sizeof(EC800M_buf))	EC800M_cnt = 0; //��ֹ���ڱ�ˢ��
		EC800M_buf[EC800M_cnt++] = USART2->DR;
		
		USART_ClearFlag(USART2, USART_FLAG_RXNE);
	}

}
