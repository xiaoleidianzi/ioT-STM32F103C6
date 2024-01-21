/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	led.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2016-11-23
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		LED��ʼ��������LED
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��ͷ�ļ�
#include "stm32f10x.h"

//LEDͷ�ļ�
#include "led.h"


LED_STATUS led_status;


/*
************************************************************
*	�������ƣ�	Led_Init
*
*	�������ܣ�	LED��ʼ��
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		LED4-PC7	LED5-PC8	LED6-PA12	LED7-PC10
				�ߵ�ƽ�ص�		�͵�ƽ����
************************************************************
*/
void Led_Init(void)
{
	
		//	GPIO_InitTypeDef gpio_initstruct;

		//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);	//��GPIOA��GPIOC��ʱ��
		//	
		//	gpio_initstruct.GPIO_Mode = GPIO_Mode_Out_PP;									//����Ϊ�������ģʽ
		//	gpio_initstruct.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 |GPIO_Pin_10;					//��ʼ��Pin7��8��10
		//	gpio_initstruct.GPIO_Speed = GPIO_Speed_50MHz;									//���ص����Ƶ��
		//	GPIO_Init(GPIOC, &gpio_initstruct);												//��ʼ��GPIOC
		//	
		//	gpio_initstruct.GPIO_Pin = GPIO_Pin_12;											//��ʼ��Pin12
		//	GPIO_Init(GPIOA, &gpio_initstruct);												//��ʼ��GPIOA
    
    LedB12_Set(LED_OFF);
    LedB13_Set(LED_OFF);
    LedB14_Set(LED_OFF);
    LedB15_Set(LED_OFF);

}

/*
************************************************************
*	�������ƣ�	Led4_Set
*
*	�������ܣ�	LED4����
*
*	��ڲ�����	status��LED_ON-����	LED_OFF-�ص�
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void LedB12_Set(LED_ENUM status)
{

	if(status == LED_ON)
	{
		GPIO_WriteBit(GPIOB, GPIO_Pin_12,Bit_SET);	//���
		//led_status.LedF6Sta = status;
	}
	else
	{
		GPIO_WriteBit(GPIOB, GPIO_Pin_12,Bit_RESET);//�ص�
	}
}

/*
************************************************************
*	�������ƣ�	Led5_Set
*
*	�������ܣ�	LED5����
*
*	��ڲ�����	status��LED_ON-����	LED_OFF-�ص�
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void LedB13_Set(LED_ENUM status)
{

	if(status == LED_ON)
	{
		GPIO_WriteBit(GPIOB, GPIO_Pin_13,Bit_SET);	//���
		//led_status.LedF6Sta = status;
	}
	else
	{
		GPIO_WriteBit(GPIOB, GPIO_Pin_13,Bit_RESET);//�ص�
	}
}

/*
************************************************************
*	�������ƣ�	Led6_Set
*
*	�������ܣ�	LED6����
*
*	��ڲ�����	status��LED_ON-����	LED_OFF-�ص�
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void LedB14_Set(LED_ENUM status)
{

	if(status == LED_ON)
	{
		GPIO_WriteBit(GPIOB, GPIO_Pin_14,Bit_SET);	//���
		//led_status.LedF6Sta = status;
	}
	else
	{
		GPIO_WriteBit(GPIOB, GPIO_Pin_14,Bit_RESET);//�ص�
	}
}

/*
************************************************************
*	�������ƣ�	Led7_Set
*
*	�������ܣ�	LED7����
*
*	��ڲ�����	status��LED_ON-����	LED_OFF-�ص�
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void LedB15_Set(LED_ENUM status)
{

	if(status == LED_ON)
	{
		GPIO_WriteBit(GPIOB, GPIO_Pin_15,Bit_SET);	//���
		//led_status.LedF6Sta = status;
	}
	else
	{
		GPIO_WriteBit(GPIOB, GPIO_Pin_15,Bit_RESET);//�ص�
	}

}
