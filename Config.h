#ifndef __CONFIG_H__
#define __CONFIG_H__

//WIFI�˻�������
#define ESP8266_WIFI_INFO		"AT+CWJAP=\"CU-C51D\",\"13841839659\"\r\n"

//�Ʒ�����IP���˿ں�
#define ESP8266_ONENET_INFO		"AT+CIPSTART=\"TCP\",\"183.230.40.96\",1883\r\n"
//��ƷID
#define PROID			"470285"

//access_key
//#define ACCESS_KEY		"WMwRLiaVDsOGsj7r7iVrQ5cigXE60DQNgp68ylYUnmE="

//�豸key
#define ACCESS_KEY		"TmZ5csCJjqGS/Q1QaWwtY0cB+mWY3BEZQz1Ml52NiuQ="
//�豸����
#define DEVICE_NAME		"my_first_device"


extern unsigned char esp8266_buf[512];
extern double power;
extern double temperatrue;

#endif
