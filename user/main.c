/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	main.c
	*
	*	作者： 		徐磊
	*
	*	日期： 		2021-12-04
	*
	*	版本： 		V1.0
	*
	*	说明： 		接入onenet，上传数据和命令控制
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//单片机头文件
#include "stm32f10x.h"

//网络协议层
#include "onenet.h"

//网络设备
#include "esp8266.h"

//硬件驱动
#include "delay.h"
#include "led.h"
#include "beep.h"
#include "key.h"
#include "usart.h"
#include "Electromotor.h"
#include "DHT11.h"
//配置库
#include "Config.h"
//C库
#include <string.h>


GPIO_InitTypeDef GPIO_InitStructure;
DMA_InitTypeDef  DMA_InitStructure; 
ADC_InitTypeDef  ADC_InitStructure;
void GPIO_Configuration(void);
void RCC_Configuration(void);
void ADC_Init_B0(void);
vu16 ADCConvertedValue[4];
#define ADC1_DR_Address    ((u32)0x4001244C)
double MQ2=0.0;				  //气体
double MQ4=0.0;				  //烟雾

double temperatrue=0.0;	//温度

double DHT_temp=0.0;	//温度
double DHT_humi=0.0;	//湿度
/*
************************************************************
*	函数名称：	Hardware_Init
*
*	函数功能：	硬件初始化
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		初始化单片机功能以及外接设备
************************************************************
*/
void Hardware_Init(void)
{
	
	RCC_Configuration();
	
	GPIO_Configuration();
	
	ADC_Init_B0();
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//中断控制器分组设置

	Delay_Init();									//systick初始化
	
	Usart1_Init(115200);							//串口1，打印信息用
	
	Usart2_Init(115200);							//串口2，驱动ESP8266用
	
	Led_Init();										//LED初始化
	
	//Electromotor_Init();							//电机驱动初始化

	Beep_Init();									//蜂鸣器初始化
	
	//Key_Init();										//按键初始化
	
	DHT11_Init();									//温湿度传感器初始化
	
	
	UsartPrintf(USART_DEBUG, " Hardware init OK\r\n");
	
}

/*
************************************************************
*	函数名称：	main
*
*	函数功能：	
*
*	入口参数：	无
*
*	返回参数：	0
*
*	说明：		
************************************************************
*/
int main(void)
{
		
	unsigned short timeCount = 0;	//发送间隔变量	
	unsigned char *dataPtr = NULL;
	char temp[2];
	char humi[2];
	
	Hardware_Init();					//初始化外围硬件
	ESP8266_Init();						//初始化ESP8266	
	//OneNET_RegisterDevice();	//设备注册，通过算法获取登录KEY	
	UsartPrintf(USART_DEBUG, "Connect MQTTs Server...\r\n");
	
	while(ESP8266_SendCmd(ESP8266_ONENET_INFO, "CONNECT")) //接入OneNET MQTTS服务器
		DelayXms(500);
	
	while(OneNet_DevLink())			//接入OneNET
		DelayXms(500);
	
	OneNET_Subscribe("$sys/%s/%s/cmd/#", PROID, DEVICE_NAME);					//向平台发送订阅请求(便于后续接收系统下发指令)
	
	//Electromotor_Enable_Set(Electromotor_ON);
	//Electromotor_Direction_Set(Electromotor_ON);
	//Beep_Set(BEEP_ON);
	//DelayXms(500);
	//Beep_Set(BEEP_OFF);
	while(1)
	{
		
		//数据处理
		DHTll_Read_Data(temp, humi);										//读取温湿度传感器数据
		
		DHT_temp=(double)temp[0]+(double)temp[1]/10.0;
		DHT_humi=(double)humi[0]+(double)humi[1]/10.0;
		MQ2 =  (double)ADCConvertedValue[0]/4096.0-0.01; //气体浓度
		MQ4 =  (double)ADCConvertedValue[2]/4096.0-0.01; //气体浓度
		
		if((MQ2 > 0.4)||(MQ4 > 0.4))
		{
				Beep_Set(BEEP_ON);
		}
		else
		{
				Beep_Set(BEEP_OFF);
		}
		
		temperatrue =(1.43-(double)ADCConvertedValue[1]*3.3/4095)/0.0043+25 ;	//返回最近一次ADC1规则组的转换结果
		//LED灯状态获取
		led_status.LedB12Sta = GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12);
		led_status.LedB13Sta = GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13);
		led_status.LedB14Sta = GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14);
		led_status.LedB15Sta = GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15);	
		
		
		if(++timeCount >= 500)									//发送间隔5s
		{
			
			UsartPrintf(USART_DEBUG, "OneNet_SendData\r\n");			
			OneNet_SendData();									//向平台发送数据			
			DelayXms(10);			
			//OneNet_ping();									  //心跳请求		
			timeCount = 0;
			ESP8266_Clear();										//清空缓存
		}
		
		/*if(timeCount%2==0)									//发送间隔5s
		{
			Electromotor_Pulse_Set(Electromotor_ON);	
		}
		else
		{
			Electromotor_Pulse_Set(Electromotor_OFF);
		}
		*/			
		dataPtr = ESP8266_GetIPD(0);					//获取ONENET平台返回的数据
		if(dataPtr != NULL)
			OneNet_RevPro(dataPtr);							//解析ONENET数据
		
		DelayXms(10);	
	}
}

void RCC_Configuration(void)
{   

		ErrorStatus HSEstatue;
		//1 复位时钟
		RCC_DeInit();
	
		//2 HSE使能并等待其就绪
		RCC_HSEConfig(RCC_HSE_ON);
		HSEstatue = RCC_WaitForHSEStartUp();
	
		if(HSEstatue==SUCCESS)
		{
			//3 HSE使能预取值，配置等待周期
			FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
			FLASH_SetLatency(FLASH_Latency_2);
			
			//4 配置时钟来源和倍频系数8M×9=72M
			RCC_PLLConfig(RCC_PLLSource_HSE_Div1,RCC_PLLMul_9);
			
			//5 使能PLL并等待其稳定
			RCC_PLLCmd(ENABLE);
			while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY)==RESET);
			
			//6 选择系统时钟
			RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
			while(RCC_GetSYSCLKSource() != 0x08);
			
			//7 设置HCLK,PCLK2,PCLK2时钟
			RCC_HCLKConfig(RCC_SYSCLK_Div1);
			//8 设置APB1
			RCC_PCLK1Config(RCC_HCLK_Div2);
			//9 设置APB2
			RCC_PCLK2Config(RCC_HCLK_Div1);
			
		}
		else
		{
			//配置错误执行的代码块
		}

      /* Enable DMA1 clock */
      RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
      /* Enable ADC1, ADC2, ADC3 and GPIOC clocks */
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 |RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOF | RCC_APB2Periph_ADC1 , ENABLE);
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
			//GPIO重映射
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
		//内部时钟配置
		
//    RCC_DeInit();
//    // Enable HSI 
//    RCC_HSICmd(ENABLE);
//    //Wait till HSE is ready 
//    while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET){;}
//    if(1)
//    {
//      /* Enable Prefetch Buffer */
//      //FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

//      /* Flash 2 wait state */
//      //FLASH_SetLatency(FLASH_Latency_2);
// 
//      /* HCLK = SYSCLK */
//      RCC_HCLKConfig(RCC_SYSCLK_Div1); 
//  
//      /* PCLK2 = HCLK */
//      RCC_PCLK2Config(RCC_HCLK_Div1); 

//      /* PCLK1 = HCLK/2 */
//      RCC_PCLK1Config(RCC_HCLK_Div2);
//      /* ADCCLK = PCLK2/4 */
//      RCC_ADCCLKConfig(RCC_PCLK2_Div4);

//      /* PLLCLK = 8MHz /2* 14 = 64 MHz 内部晶振*/
//      RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_14);//RCC_PLLMul_14
//      /* Enable PLL */ 
//      RCC_PLLCmd(ENABLE);

//      /* Wait till PLL is ready */
//      while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
//      {
//      }
//      /* Select PLL as system clock source */
//      RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
//  
//      /* Wait till PLL is used as system clock source */
//      while(RCC_GetSYSCLKSource() != 0x08)
//      {
//      }
//    }

}

void GPIO_Configuration(void)
{

    //GPIO_PinRemapConfig(GPIO_Remap_USART2, ENABLE);
    //GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);
		
		//GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);//USART1_REMAP = 0 PA9/PA10
		GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);
		//GPIO_Pin
    
    //GPIO_InitTypeDef GPIO_InitStructure;
     /* Configure USART1 Tx (PA.09) as alternate function push-pull */
//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;//GPIO_Mode_AF_PP;//GPIO_Mode_Out_PP;
//    GPIO_Init(GPIOA, &GPIO_InitStructure);

//    /* Configure USART1 Rx (PA.10) as input floating */
//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
		//UART1重映射 PB6、PB7
		/* Configure USART1 Tx (PB.6) as input floating */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;//GPIO_Mode_AF_PP;//GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Configure USART1 Rx (PB.7) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
		
      /* Configure USART2 Tx (PA.02) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Configure USART2 Rx (PA.03) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    //LED B12/13/14/15
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
   
    /*SPI模拟接口*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;//miso
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;//sck
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//mosi
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
     
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;//nss
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;//reset
    //GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
		//ADC电压输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;//reset
    //GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    
		//ESP8266复位引脚
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;					//GPIOB8-复位
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	  //步进电机控制
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;					//GPIOC13-使能
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOC, &GPIO_InitStructure);
		
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;					//GPIOC14-方向
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOC, &GPIO_InitStructure);
		
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;					//GPIOC15-脉冲
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOC, &GPIO_InitStructure);
                 //SWJ_CFG
    //AFIO->MAPR =(0x02<<24);
    
    //GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST, ENABLE);
    // Configure PB.03 (JTDO) and PB.04 (JTRST) as output push-pull 

}


void ADC_Init_B0(void)
{
			//NVIC_InitTypeDef nvicInitStruct;
     /* DMA1 channel1 configuration ----------------------------------------------*/
      DMA_DeInit(DMA1_Channel1);
      DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;				//DMA外设基地址
      DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&ADCConvertedValue;		//定义DMA内存基地址
      DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;								//外设作为数据传输的来源
      DMA_InitStructure.DMA_BufferSize = 4;															//指定DMA1通道的DMA缓存的大小
      DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	//外设地址寄存器不变
      DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;						//内存地址寄存器递增
      DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//数据宽度为 16 位
      DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;//数据宽度为 16 位
      DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;										//工作在循环缓存模式
      DMA_InitStructure.DMA_Priority = DMA_Priority_High;								//DMA通道1拥有高优先级
      DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;											//DMA通道1没有设置为内存到内存传输
      DMA_Init(DMA1_Channel1, &DMA_InitStructure);
      
      /* Enable DMA1 channel1 */
      DMA_Cmd(DMA1_Channel1, ENABLE);
      DMA_ITConfig(DMA1_Channel1,DMA_IT_TC,ENABLE);   
      /* ADC1 configuration ------------------------------------------------------*/
      //RCC_ADCCLKConfig(RCC_PCLK2_Div8);
	
			ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;			//独立工作模式 
      ADC_InitStructure.ADC_ScanConvMode = ENABLE;						//扫描方式
      ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;			//连续转换
      ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;//外部触发禁止
      ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//数据右对齐
      ADC_InitStructure.ADC_NbrOfChannel = 4;									//用于转换的通道数
      ADC_Init(ADC1, &ADC_InitStructure);											//配置上述参数
      ADC_TempSensorVrefintCmd(ENABLE);												//使能温度采集通道
      /* ADC1 regular channel14 configuration */ 
			//规则模式通道配置
      ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5);	//气体浓度采集
      ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 2, ADC_SampleTime_239Cycles5); //温度采集
			ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 3, ADC_SampleTime_239Cycles5);	//烟雾浓度采集
      //ADC_RegularChannelConfig(ADC1, ADC_Channel_17, 3, ADC_SampleTime_239Cycles5);
      /* Enable ADC1 DMA */
      ADC_DMACmd(ADC1, ENABLE);
      
			/* 配置为转换结束后产生中断 在中断中读取信息 */
			//ADC_ITConfig(ADC1, ADC_IT_EOC,ENABLE);
	
      /* Enable ADC1 */
      ADC_Cmd(ADC1, ENABLE);
    
      /* Enable ADC1 reset calibaration register */ 
			//使能ADC1复位校准寄存器			
      ADC_ResetCalibration(ADC1);
      /* Check the end of ADC1 reset calibration register */
			//检查校准寄存器是否复位完毕
      while(ADC_GetResetCalibrationStatus(ADC1));
    
      /* Start ADC1 calibaration */
			//开始校准
      ADC_StartCalibration(ADC1);
      /* Check the end of ADC1 calibration */
			//检测是否校准完毕
      while(ADC_GetCalibrationStatus(ADC1));
         
      /* Start ADC1 Software Conversion */ 
			//开启ADC1的软件转换
      ADC_SoftwareStartConvCmd(ADC1, ENABLE);
			
}

