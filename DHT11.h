#ifndef __DHT11_H
#define __DHT11_H

#define DHT11_RCC  RCC_APB2Periph_GPIOA //开启引脚RCC，根据具体情况修改//定义端口，根据具体情况修改
#define DHT11_PORT GPIOA//定义I0口，根据具体情况修改
#define DHT11_IO   GPIO_Pin_12//设置I0口为输出模式设置I0口为输入模式

void DHT11_IO_OUT  (void);
void DHT11_IO_IN(void);
void DHT11_RST(void);
char DHT11_Check(void);
char DHT11_Read_Bit(void);
char DHT11_Read_Byte(void);
//DHT11 初始化
char DHT11_Init(void);
char DHTll_Read_Data(char *temp,char *humi);

#endif
