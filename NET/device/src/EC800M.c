/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	EC800M.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2017-05-08
	*
	*	版本： 		V1.0
	*
	*	说明： 		EC800M的简单驱动
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//单片机头文件
#include "stm32f10x.h"

//网络设备驱动
#include "EC800M.h"

//硬件驱动
#include "delay.h"
#include "usart.h"

//C库
#include <string.h>
#include <stdio.h>

//加载配置文件
#include "Config.h"


unsigned char EC800M_buf[512];
unsigned short EC800M_cnt = 0, EC800M_cntPre = 0;


//==========================================================
//	函数名称：	EC800M_Clear
//
//	函数功能：	清空缓存
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void EC800M_Clear(void)
{

	memset(EC800M_buf, 0, sizeof(EC800M_buf));
	EC800M_cnt = 0;

}

//==========================================================
//	函数名称：	EC800M_WaitRecive
//
//	函数功能：	等待接收完成
//
//	入口参数：	无
//
//	返回参数：	REV_OK-接收完成		REV_WAIT-接收超时未完成
//
//	说明：		循环调用检测是否接收完成
//==========================================================
_Bool EC800M_WaitRecive(void)
{

	if(EC800M_cnt == 0) 							//如果接收计数为0 则说明没有处于接收数据中，所以直接跳出，结束函数
		return REV_WAIT;
		
	if(EC800M_cnt == EC800M_cntPre)				//如果上一次的值和这次相同，则说明接收完毕
	{
		EC800M_cnt = 0;							//清0接收计数
			
		return REV_OK;								//返回接收完成标志
	}
		
	EC800M_cntPre = EC800M_cnt;					//置为相同
	
	return REV_WAIT;								//返回接收未完成标志

}

//==========================================================
//	函数名称：	EC800M_SendCmd
//
//	函数功能：	发送命令
//
//	入口参数：	cmd：命令
//				res：需要检查的返回指令
//
//	返回参数：	0-成功	1-失败
//
//	说明：		
//==========================================================
_Bool EC800M_SendCmd(char *cmd, char *res)
{
	
	unsigned char timeOut = 200;

	Usart_SendString(USART2, cmd, strlen((const char *)cmd));

	while(timeOut--)
	{
		if(EC800M_WaitRecive() == REV_OK)							//如果收到数据
		{
			if(strstr((const char *)EC800M_buf, res) != NULL)		//如果检索到关键词
			{
				EC800M_Clear();									//清空缓存
				
				return 0;
			}
		}
		
		DelayXms(10);
	}
	
	return 1;

}

//==========================================================
//	函数名称：	EC800M_SendData
//
//	函数功能：	发送数据
//
//	入口参数：	data：数据
//				len：长度
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void EC800M_SendData(unsigned char *data, unsigned short len)
{

	char cmdBuf[32];
	
	EC800M_Clear();								//清空接收缓存
	sprintf(cmdBuf, "AT+CIPSEND=%d\r\n", len);		//发送命令
	if(!EC800M_SendCmd(cmdBuf, ">"))				//收到‘>’时可以发送数据
	{
		Usart_SendString(USART2, data, len);		//发送设备连接请求数据
	}

}

//==========================================================
//	函数名称：	EC800M_GetIPD
//
//	函数功能：	获取平台返回的数据
//
//	入口参数：	等待的时间(乘以10ms)
//
//	返回参数：	平台返回的原始数据
//
//	说明：		不同网络设备返回的格式不同，需要去调试
//				如EC800M的返回格式为	"+IPD,x:yyy"	x代表数据长度，yyy是数据内容
//==========================================================
unsigned char *EC800M_GetIPD(unsigned short timeOut)
{

	char *ptrIPD = NULL;
	
	do
	{
		if(EC800M_WaitRecive() == REV_OK)								//如果接收完成
		{
			ptrIPD = strstr((char *)EC800M_buf, "IPD,");				//搜索“IPD”头
			if(ptrIPD == NULL)											//如果没找到，可能是IPD头的延迟，还是需要等待一会，但不会超过设定的时间
			{
				//UsartPrintf(USART_DEBUG, "\"IPD\" not found\r\n");
			}
			else
			{
				ptrIPD = strchr(ptrIPD, ':');							//找到':'
				if(ptrIPD != NULL)
				{
					ptrIPD++;
					return (unsigned char *)(ptrIPD);
				}
				else
					return NULL;
				
			}
		}
		
		DelayXms(5);													//延时等待
	} while(timeOut--);
	
	return NULL;														//超时还未找到，返回空指针

}

//==========================================================
//	函数名称：	EC800M_Init
//
//	函数功能：	初始化EC800M
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
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
	while(EC800M_SendCmd(EC800M_DEVICE_INFO,"OK")) //设备注册
		DelayXms(500);
	
	UsartPrintf(USART_DEBUG, "6. EC800M Init OK\r\n");

}

//==========================================================
//	函数名称：	USART2_IRQHandler
//
//	函数功能：	串口2收发中断
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void EC800M_USART2_IRQHandler(void)
{

	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) //接收中断
	{
		if(EC800M_cnt >= sizeof(EC800M_buf))	EC800M_cnt = 0; //防止串口被刷爆
		EC800M_buf[EC800M_cnt++] = USART2->DR;
		
		USART_ClearFlag(USART2, USART_FLAG_RXNE);
	}

}
