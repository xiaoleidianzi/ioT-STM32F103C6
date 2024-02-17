#ifndef __DHT11_H
#define __DHT11_H

#define DHT11_RCC  RCC_APB2Periph_GPIOA //��������RCC�����ݾ�������޸�//����˿ڣ����ݾ�������޸�
#define DHT11_PORT GPIOA//����I0�ڣ����ݾ�������޸�
#define DHT11_IO   GPIO_Pin_12//����I0��Ϊ���ģʽ����I0��Ϊ����ģʽ

void DHT11_IO_OUT  (void);
void DHT11_IO_IN(void);
void DHT11_RST(void);
char DHT11_Check(void);
char DHT11_Read_Bit(void);
char DHT11_Read_Byte(void);
//DHT11 ��ʼ��
char DHT11_Init(void);
char DHTll_Read_Data(char *temp,char *humi);

#endif
