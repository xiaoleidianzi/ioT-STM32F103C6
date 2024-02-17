#include "stm32f10x.h"                  // Device header
#include "DHT11.h"
#include "Delay.h"

void DHT11_IO_OUT (void){ //��ʪ��ģ���������
	
	GPIO_InitTypeDef  GPIO_InitStructure; 	
  GPIO_InitStructure.GPIO_Pin = DHT11_IO; //ѡ��˿ںţ�0~15��all��                        
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //�������       
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //����IO�ӿ��ٶȣ�2/10/50MHz��    
	GPIO_Init(DHT11_PORT, &GPIO_InitStructure);
}

void DHT11_IO_IN (void){ //��ʪ��ģ�����뺯��
	GPIO_InitTypeDef  GPIO_InitStructure; 	
  GPIO_InitStructure.GPIO_Pin = DHT11_IO; //ѡ��˿ںţ�0~15��all��                        
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; //��������      
	GPIO_Init(DHT11_PORT, &GPIO_InitStructure);
}

void DHT11_RST (void){ 						//DHT11�˿ڸ�λ��������ʼ�źţ�IO���ͣ�
	DHT11_IO_OUT();							//�˿�Ϊ���
	GPIO_ResetBits(DHT11_PORT,DHT11_IO); 	//ʹ����Ϊ�͵�ƽ
	DelayXms(20); 							//��������18ms						
	GPIO_SetBits(DHT11_PORT,DHT11_IO); 		//ʹ����Ϊ�ߵ�ƽ							
	DelayUs(30); 							//��������20~40us
}

char DHT11_Check(void){ 	//�ȴ�DHT11��Ӧ������1:δ��⵽DHT11������0:�ɹ���IO���գ�	   
    char retry=0;			//������ʱ����
    DHT11_IO_IN();		//IO������״̬	 
//GPIO�˿�����ʱ������Ϊ����������߸������룬��Ϊ����������裬����Ĭ��Ϊ�ߵ�ƽ
//���DHT11������������Ϊ�ߵ�ƽ���� retry С��100���� retry ��1������ʱ1΢�룬�ظ��������ֱ�� retry ���ڵ���100 ����DHT11�������������ɵ͵�ƽ����� retry ���ڵ���100����ʾ���ʧ�ܣ�����1�����򣬽� retry ����Ϊ0��
	while ((GPIO_ReadInputDataBit(DHT11_PORT,DHT11_IO) == 1) && retry<100)	//DHT11������40~80us
	{
		retry++;
        DelayUs(1);
    }
    if(retry>=100)return 1; 	
	else retry=0;
//���DHT11������������Ϊ�͵�ƽ���� retry С��100���� retry ��1������ʱ1΢�룬�ظ��������ֱ�� retry ���ڵ���100 ����DHT11�������������ɸߵ�ƽ����� retry ���ڵ���100����ʾ���ʧ�ܣ�����1�����򣬷���0����ʾ���ɹ���
    while ((GPIO_ReadInputDataBit(DHT11_PORT,DHT11_IO) == 0) && retry<100)  //DHT11���ͺ���ٴ�����40~80us
	{  
        retry++;
        DelayUs(1);
    }
    if(retry>=100)return 1;	    
    return 0;
}

char DHT11_Init(void){	//DHT11��ʼ��
	RCC_APB2PeriphClockCmd(DHT11_RCC,ENABLE);	//��ʼDHT11��ʱ��
	DHT11_RST();								//DHT11�˿ڸ�λ��������ʼ�ź�
	return DHT11_Check(); 						//�ȴ�DHT11��Ӧ
}

//��DHT11��ȡһ��λ
//����ֵ��1/0
char DHT11_Read_Bit(void)
{
    char retry = 0;
    while((GPIO_ReadInputDataBit(DHT11_PORT,DHT11_IO) == 1) && retry < 100) //�ȴ���Ϊ�͵�ƽ
    {
        retry++;
        DelayUs(1);
    }
    retry = 0;
    while((GPIO_ReadInputDataBit(DHT11_PORT,DHT11_IO) == 0) && retry < 100) //�ȴ���ߵ�ƽ
    {
        retry++;
        DelayUs(1);
    }
    DelayUs(40);//�ȴ�40us
    if(GPIO_ReadInputDataBit(DHT11_PORT,DHT11_IO) == 1)       //�����жϸߵ͵�ƽ��������1��0
        return 1;
    else
        return 0;
}

//��DHT11��ȡһ���ֽ�
//����ֵ������������
char DHT11_Read_Byte(void)
{
    char i, dat;
    dat = 0;
    for (i = 0; i < 8; i++)
    {
        dat <<= 1;					//���������,dat����1λ
        dat |= DHT11_Read_Bit();	//"|"��ʾ��λ�����
    }
    return dat;
}

//��DHT11��ȡһ������
//temp:�¶�ֵ(��Χ:0~50��)
//humi:ʪ��ֵ(��Χ:20%~90%)
//����ֵ��0,����;1,��ȡʧ��
char DHTll_Read_Data(char *temp, char *humi)
{
    char buf[5];
    char i;
    DHT11_RST();												//DHT11�˿ڸ�λ��������ʼ�ź�
    if(DHT11_Check() == 0)							//�ȴ�DHT11��Ӧ��0Ϊ�ɹ���Ӧ
    {
        for(i = 0; i < 5; i++) 					//��ȡ40λ����
        {
            buf[i] = DHT11_Read_Byte();	//��������
        }
        if((buf[0] + buf[1] + buf[2] + buf[3]) == buf[4])	//����У��
        {
            *humi = buf[0];							//��ʪ��ֵ����ָ��humi
            *temp = buf[2];							//���¶�ֵ����ָ��temp
        }
    }
    else return 1;
    return 0;
}


