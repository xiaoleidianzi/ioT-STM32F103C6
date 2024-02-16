//��Ƭ��ͷ�ļ�
#include "stm32f10x.h"

//LEDͷ�ļ�
#include "Electromotor.h"

void Electromotor_Init(void)
{
    Electromotor_Enable_Set(Electromotor_OFF);
    Electromotor_Direction_Set(Electromotor_OFF);
    Electromotor_Pulse_Set(Electromotor_OFF);
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
void Electromotor_Enable_Set(Electromotor_ENUM status)
{

	if(status == Electromotor_ON)
	{
		GPIO_WriteBit(GPIOC, GPIO_Pin_15,Bit_SET);	//���
		//led_status.LedF6Sta = status;
	}
	else
	{
		GPIO_WriteBit(GPIOC, GPIO_Pin_15,Bit_RESET);//�ص�
	}

}

void Electromotor_Direction_Set(Electromotor_ENUM status)
{

	if(status == Electromotor_ON)
	{
		GPIO_WriteBit(GPIOC, GPIO_Pin_14,Bit_SET);	//���
		//led_status.LedF6Sta = status;
	}
	else
	{
		GPIO_WriteBit(GPIOC, GPIO_Pin_14,Bit_RESET);//�ص�
	}

}

void Electromotor_Pulse_Set(Electromotor_ENUM status)
{

	if(status == Electromotor_ON)
	{
		GPIO_WriteBit(GPIOC, GPIO_Pin_13,Bit_SET);	//���
		//led_status.LedF6Sta = status;
	}
	else
	{
		GPIO_WriteBit(GPIOC, GPIO_Pin_13,Bit_RESET);//�ص�
	}

}

