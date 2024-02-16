/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	onenet.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2017-05-08
	*
	*	�汾�� 		V1.1
	*
	*	˵���� 		��onenetƽ̨�����ݽ����ӿڲ�
	*
	*	�޸ļ�¼��	V1.0��Э���װ�������ж϶���ͬһ���ļ������Ҳ�ͬЭ��ӿڲ�ͬ��
	*				V1.1���ṩͳһ�ӿڹ�Ӧ�ò�ʹ�ã����ݲ�ͬЭ���ļ�����װЭ����ص����ݡ�
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��ͷ�ļ�
#include "stm32f10x.h"

//�����豸
#include "esp8266.h"

//Э���ļ�
#include "onenet.h"
#include "mqttkit.h"

//�㷨
#include "base64.h"
#include "hmac_sha1.h"

//Ӳ������
#include "usart.h"
#include "delay.h"
#include "led.h"

//C��
#include <string.h>
#include <stdio.h>

//���������ļ�
#include "Config.h"

#include <stdarg.h>
//#define AUTH_INFO	"my_first_device"

//#define DEVID		"855470753"


char devid[16];

char key[48];


extern unsigned char esp8266_buf[512];
extern double power;
extern double temperatrue;

/*
************************************************************
*	�������ƣ�	OTA_UrlEncode
*
*	�������ܣ�	sign��Ҫ����URL����
*
*	��ڲ�����	sign�����ܽ��
*
*	���ز�����	0-�ɹ�	����-ʧ��
*
*	˵����		+			%2B
*				�ո�		%20
*				/			%2F
*				?			%3F
*				%			%25
*				#			%23
*				&			%26
*				=			%3D
************************************************************
*/
static unsigned char OTA_UrlEncode(char *sign)
{

	char sign_t[40];
	unsigned char i = 0, j = 0;
	unsigned char sign_len = strlen(sign);
	
	if(sign == (void *)0 || sign_len < 28)
		return 1;
	
	for(; i < sign_len; i++)
	{
		sign_t[i] = sign[i];
		sign[i] = 0;
	}
	sign_t[i] = 0;
	
	for(i = 0, j = 0; i < sign_len; i++)
	{
		switch(sign_t[i])
		{
			case '+':
				strcat(sign + j, "%2B");j += 3;
			break;
			
			case ' ':
				strcat(sign + j, "%20");j += 3;
			break;
			
			case '/':
				strcat(sign + j, "%2F");j += 3;
			break;
			
			case '?':
				strcat(sign + j, "%3F");j += 3;
			break;
			
			case '%':
				strcat(sign + j, "%25");j += 3;
			break;
			
			case '#':
				strcat(sign + j, "%23");j += 3;
			break;
			
			case '&':
				strcat(sign + j, "%26");j += 3;
			break;
			
			case '=':
				strcat(sign + j, "%3D");j += 3;
			break;
			
			default:
				sign[j] = sign_t[i];j++;
			break;
		}
	}
	
	sign[j] = 0;
	
	return 0;

}

/*
************************************************************
*	�������ƣ�	OTA_Authorization
*
*	�������ܣ�	����Authorization
*
*	��ڲ�����	ver��������汾�ţ����ڸ�ʽ��Ŀǰ��֧�ָ�ʽ"2018-10-31"
*				res����Ʒid
*				et������ʱ�䣬UTC��ֵ
*				access_key��������Կ
*				dev_name���豸��
*				authorization_buf������token��ָ��
*				authorization_buf_len������������(�ֽ�)
*
*	���ز�����	0-�ɹ�	����-ʧ��
*
*	˵����		��ǰ��֧��sha1
************************************************************
*/
#define METHOD		"sha1"
static unsigned char OneNET_Authorization(char *ver, char *res, unsigned int et, char *access_key, char *dev_name,
											char *authorization_buf, unsigned short authorization_buf_len, _Bool flag)
{
	
	size_t olen = 0;
	
	char sign_buf[64];								//����ǩ����Base64������ �� URL������
	char hmac_sha1_buf[64];							//����ǩ��
	char access_key_base64[64];						//����access_key��Base64������
	char string_for_signature[72];					//����string_for_signature������Ǽ��ܵ�key

//----------------------------------------------------�����Ϸ���--------------------------------------------------------------------
	if(ver == (void *)0 || res == (void *)0 || et < 1564562581 || access_key == (void *)0
		|| authorization_buf == (void *)0 || authorization_buf_len < 120)
		return 1;
	
//----------------------------------------------------��access_key����Base64����----------------------------------------------------
	memset(access_key_base64, 0, sizeof(access_key_base64));
	BASE64_Decode((unsigned char *)access_key_base64, sizeof(access_key_base64), &olen, (unsigned char *)access_key, strlen(access_key));
	//UsartPrintf(USART_DEBUG, "access_key_base64: %s\r\n", access_key_base64);
	
//----------------------------------------------------����string_for_signature-----------------------------------------------------
	memset(string_for_signature, 0, sizeof(string_for_signature));
	if(flag)
		snprintf(string_for_signature, sizeof(string_for_signature), "%d\n%s\nproducts/%s\n%s", et, METHOD, res, ver);
	else
		snprintf(string_for_signature, sizeof(string_for_signature), "%d\n%s\nproducts/%s/devices/%s\n%s", et, METHOD, res, dev_name, ver);
	//UsartPrintf(USART_DEBUG, "string_for_signature: %s\r\n", string_for_signature);
	
//----------------------------------------------------����-------------------------------------------------------------------------
	memset(hmac_sha1_buf, 0, sizeof(hmac_sha1_buf));
	
	hmac_sha1((unsigned char *)access_key_base64, strlen(access_key_base64),
				(unsigned char *)string_for_signature, strlen(string_for_signature),
				(unsigned char *)hmac_sha1_buf);
	
	//UsartPrintf(USART_DEBUG, "hmac_sha1_buf: %s\r\n", hmac_sha1_buf);
	
//----------------------------------------------------�����ܽ������Base64����------------------------------------------------------
	olen = 0;
	memset(sign_buf, 0, sizeof(sign_buf));
	BASE64_Encode((unsigned char *)sign_buf, sizeof(sign_buf), &olen, (unsigned char *)hmac_sha1_buf, strlen(hmac_sha1_buf));

//----------------------------------------------------��Base64����������URL����---------------------------------------------------
	OTA_UrlEncode(sign_buf);
	//UsartPrintf(USART_DEBUG, "sign_buf: %s\r\n", sign_buf);
	
//----------------------------------------------------����Token--------------------------------------------------------------------
	if(flag)
		snprintf(authorization_buf, authorization_buf_len, "version=%s&res=products%%2F%s&et=%d&method=%s&sign=%s", ver, res, et, METHOD, sign_buf);
	else
		snprintf(authorization_buf, authorization_buf_len, "version=%s&res=products%%2F%s%%2Fdevices%%2F%s&et=%d&method=%s&sign=%s", ver, res, dev_name, et, METHOD, sign_buf);
	//UsartPrintf(USART_DEBUG, "Token: %s\r\n", authorization_buf);
	
	return 0;

}

//==========================================================
//	�������ƣ�	OneNET_RegisterDevice
//
//	�������ܣ�	�ڲ�Ʒ��ע��һ���豸
//
//	��ڲ�����	access_key��������Կ
//				pro_id����ƷID
//				serial��Ψһ�豸��
//				devid�����淵�ص�devid
//				key�����淵�ص�key
//
//	���ز�����	0-�ɹ�		1-ʧ��
//
//	˵����		
//==========================================================
_Bool OneNET_RegisterDevice(void)
{

	_Bool result = 1;
	unsigned short send_len = 11 + strlen(DEVICE_NAME);
	char *send_ptr = NULL, *data_ptr = NULL;
	
	char authorization_buf[144];													//���ܵ�key
	
	send_ptr = malloc(send_len + 240);
	if(send_ptr == NULL)
		return result;
	
	while(ESP8266_SendCmd("AT+CIPSTART=\"TCP\",\"183.230.40.33\",80\r\n", "CONNECT"))
		DelayXms(500);
	
	OneNET_Authorization("2018-10-31", PROID, 2056499200, ACCESS_KEY, NULL,
							authorization_buf, sizeof(authorization_buf), 1);//1956499200
	
	snprintf(send_ptr, 240 + send_len, "POST /mqtt/v1/devices/reg HTTP/1.1\r\n"
					"Authorization:%s\r\n"
					"Host:ota.heclouds.com\r\n"
					"Content-Type:application/json\r\n"
					"Content-Length:%d\r\n\r\n"
					"{\"name\":\"%s\"}",
	
					authorization_buf, 11 + strlen(DEVICE_NAME), DEVICE_NAME);
	
	UsartPrintf(USART_DEBUG, send_ptr);
	
	ESP8266_SendData((char *)send_ptr, strlen(send_ptr));
	
	/*
	{
	  "request_id" : "f55a5a37-36e4-43a6-905c-cc8f958437b0",
	  "code" : "onenet_common_success",
	  "code_no" : "000000",
	  "message" : null,
	  "data" : {
		"device_id" : "589804481",
		"name" : "mcu_id_43057127",
		
	"pid" : 282932,
		"key" : "indu/peTFlsgQGL060Gp7GhJOn9DnuRecadrybv9/XY="
	  }
	}
	*/
	
	data_ptr = (char *)ESP8266_GetIPD(250);							//�ȴ�ƽ̨��Ӧ
	
	if(data_ptr)
	{
		data_ptr = strstr(data_ptr, "device_id");
	}
	
	if(data_ptr)
	{
		char name[16];
		int pid = 0;
		
		if(sscanf(data_ptr, "device_id\" : \"%[^\"]\",\r\n\"name\" : \"%[^\"]\",\r\n\r\n\"pid\" : %d,\r\n\"key\" : \"%[^\"]\"", devid, name, &pid, key) == 4)
		{
			UsartPrintf(USART_DEBUG, "create device: %s, %s, %d, %s\r\n", devid, name, pid, key);
			result = 0;
		}
	}
	
	free(send_ptr);//�ͷ��ڴ�
	ESP8266_SendCmd("AT+CIPCLOSE\r\n", "OK");
	
	return result;

}

//==========================================================
//	�������ƣ�	OneNet_DevLink
//
//	�������ܣ�	��onenet��������
//
//	��ڲ�����	��
//
//	���ز�����	1-ʧ��	0-�ɹ�
//
//	˵����		��onenetƽ̨��������
//==========================================================
_Bool OneNet_DevLink(void)
{
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};					//Э���

  char *dataPtr;
	
	char authorization_buf[160];
	
	_Bool status = 1;
	
	/*OneNET_Authorization("2018-10-31", PROID, 1956499200, key, DEVICE_NAME,
								authorization_buf, sizeof(authorization_buf), 0);*/
	
	OneNET_Authorization("2018-10-31", PROID, 1956499200, ACCESS_KEY, DEVICE_NAME,
								authorization_buf, sizeof(authorization_buf), 0);
	
	
	UsartPrintf(USART_DEBUG, "OneNET_DevLink\r\n"
							"NAME: %s,	PROID: %s,	KEY:%s\r\n"
                        , DEVICE_NAME, PROID, authorization_buf);
	
	if(MQTT_PacketConnect(PROID, authorization_buf, DEVICE_NAME, 256, 1, MQTT_QOS_LEVEL0, NULL, NULL, 0, &mqttPacket) == 0)
	{
		ESP8266_SendData(mqttPacket._data, mqttPacket._len);			//�ϴ�ƽ̨
		
		dataPtr = ESP8266_GetIPD(250);									//�ȴ�ƽ̨��Ӧ
		if(dataPtr != NULL)
		{
			if(MQTT_UnPacketRecv(dataPtr) == MQTT_PKT_CONNACK)
			{
				switch(MQTT_UnPacketConnectAck(dataPtr))
				{
					case 0:UsartPrintf(USART_DEBUG, "Tips:	���ӳɹ�\r\n");status = 0;break;
					
					case 1:UsartPrintf(USART_DEBUG, "WARN:	����ʧ�ܣ�Э�����\r\n");break;
					case 2:UsartPrintf(USART_DEBUG, "WARN:	����ʧ�ܣ��Ƿ���clientid\r\n");break;
					case 3:UsartPrintf(USART_DEBUG, "WARN:	����ʧ�ܣ�������ʧ��\r\n");break;
					case 4:UsartPrintf(USART_DEBUG, "WARN:	����ʧ�ܣ��û������������\r\n");break;
					case 5:UsartPrintf(USART_DEBUG, "WARN:	����ʧ�ܣ��Ƿ�����(����token�Ƿ�)\r\n");break;
					
					default:UsartPrintf(USART_DEBUG, "ERR:	����ʧ�ܣ�δ֪����\r\n");break;
				}
			}
		}
		
		MQTT_DeleteBuffer(&mqttPacket);								//ɾ��
	}
	else
		UsartPrintf(USART_DEBUG, "WARN:	MQTT_PacketConnect Failed\r\n");
	
	return status;
	
}

unsigned char OneNet_FillBuf(char *buf)
{
	
	char text[32];
	
	memset(text, 0, sizeof(text));
	
	strcpy(buf, "{\"id\":123,\"dp\":{");
	
	memset(text, 0, sizeof(text));
	sprintf(text, "\"temperatrue\":[{\"v\":%3f}],",temperatrue); 
	//sprintf(text, "\"Red_Led\":[{\"v\":%d}],", led_status.Led4Sta);
	strcat(buf, text);
	
	memset(text, 0, sizeof(text));
	//sprintf(text, "\"power\":%3f",power); 
	sprintf(text, "\"power\":[{\"v\":%3f}],",power); 
	strcat(buf, text);
	
	memset(text, 0, sizeof(text));
	sprintf(text, "\"LedB12\":[{\"v\":%d}],", led_status.LedB12Sta);
	strcat(buf, text);
	
	memset(text, 0, sizeof(text));
	sprintf(text, "\"LedB13\":[{\"v\":%d}],", led_status.LedB13Sta);
	strcat(buf, text);
	
	memset(text, 0, sizeof(text));
	sprintf(text, "\"LedB14\":[{\"v\":%d}],", led_status.LedB14Sta);
	strcat(buf, text);
	
	memset(text, 0, sizeof(text));
	sprintf(text, "\"LedB15\":[{\"v\":%d}]", led_status.LedB15Sta);
	strcat(buf, text);
	
	strcat(buf, "}}");
	
	return strlen(buf);

}

//==========================================================
//	�������ƣ�	OneNet_SendData
//
//	�������ܣ�	�ϴ����ݵ�ƽ̨
//
//	��ڲ�����	type���������ݵĸ�ʽ
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void OneNet_SendData(void)
{
	
	//MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};												//Э���
	
	char buf[256];
	
	short body_len = 0;
	
	UsartPrintf(USART_DEBUG, "Tips:	OneNet_SendData-MQTT\r\n");
	
	memset(buf, 0, sizeof(buf));
	
	body_len = OneNet_FillBuf(buf);																	//��ȡ��ǰ��Ҫ���͵����������ܳ���
	
	
	ESP8266_SendData(buf, body_len);
	//ESP8266_SendCmd("AT+QMTSUB=0,1,\"$sys/470285/my_first_device/cmd/#\",0\r\n","OK")//��Ϣ����
	//if(body_len)
	//{
	//	if(MQTT_PacketSaveData(PROID, DEVICE_NAME, body_len, NULL, &mqttPacket) == 0)				//���
	//	{
	//		for(; i < body_len; i++)
	//			mqttPacket._data[mqttPacket._len++] = buf[i];
			
	//		ESP8266_SendData(mqttPacket._data, mqttPacket._len);									//�ϴ����ݵ�ƽ̨
	//		UsartPrintf(USART_DEBUG, "Send %d Bytes\r\n", mqttPacket._len);
			
	//		MQTT_DeleteBuffer(&mqttPacket);															//ɾ��
	//	}
	//	else
	//		UsartPrintf(USART_DEBUG, "WARN:	EDP_NewBuffer Failed\r\n");
	//}
	
}

//==========================================================
//	�������ƣ�	OneNET_Publish
//
//	�������ܣ�	������Ϣ
//
//	��ڲ�����	topic������������
//				msg����Ϣ����
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void OneNET_Publish(const char *topic, const char *msg)
{

	MQTT_PACKET_STRUCTURE mqtt_packet = {NULL, 0, 0, 0};						//Э���
	
	UsartPrintf(USART_DEBUG, "Publish Topic: %s, Msg: %s\r\n", topic, msg);
	
	if(MQTT_PacketPublish(MQTT_PUBLISH_ID, topic, msg, strlen(msg), MQTT_QOS_LEVEL0, 0, 1, &mqtt_packet) == 0)
	{
		ESP8266_SendData(mqtt_packet._data, mqtt_packet._len);					//��ƽ̨���Ͷ�������
		
		MQTT_DeleteBuffer(&mqtt_packet);										//ɾ��
	}

}

//==========================================================
//	�������ƣ�	OneNET_Subscribe
//
//	�������ܣ�	����
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void OneNET_Subscribe(const char * topic,...)
{
	
	MQTT_PACKET_STRUCTURE mqtt_packet = {NULL, 0, 0, 0};						//Э���
	
	char topic_buf[56];
	const char *topic1 = topic_buf;
	va_list ap;
	va_start(ap,topic);
	
	vsnprintf(topic_buf, sizeof(topic_buf),topic,ap);// "$sys/%s/%s/cmd/#", PROID, DEVICE_NAME
	
	va_end(ap);
	
	UsartPrintf(USART_DEBUG, "Subscribe Topic: %s\r\n", topic_buf);
	
	if(MQTT_PacketSubscribe(MQTT_SUBSCRIBE_ID, MQTT_QOS_LEVEL0, &topic1, 1, &mqtt_packet) == 0)
	{
		ESP8266_SendData(mqtt_packet._data, mqtt_packet._len);					//��ƽ̨���Ͷ�������
		
		MQTT_DeleteBuffer(&mqtt_packet);										//ɾ��
	}

}

////==========================================================
////	�������ƣ�	OneNET_Subscribe
////
////	�������ܣ�	����
////
////	��ڲ�����	��
////
////	���ز�����	��
////
////	˵����		
////==========================================================
//void OneNET_Subscribe(void)
//{
//	
//	MQTT_PACKET_STRUCTURE mqtt_packet = {NULL, 0, 0, 0};						//Э���
//	
//	char topic_buf[56];
//	const char *topic = topic_buf;
//	
//	snprintf(topic_buf, sizeof(topic_buf), "$sys/%s/%s/cmd/#", PROID, DEVICE_NAME);
//	
//	UsartPrintf(USART_DEBUG, "Subscribe Topic: %s\r\n", topic_buf);
//	
//	if(MQTT_PacketSubscribe(MQTT_SUBSCRIBE_ID, MQTT_QOS_LEVEL0, &topic, 1, &mqtt_packet) == 0)
//	{
//		ESP8266_SendData(mqtt_packet._data, mqtt_packet._len);					//��ƽ̨���Ͷ�������
//		
//		MQTT_DeleteBuffer(&mqtt_packet);										//ɾ��
//	}

//}

//==========================================================
//	�������ƣ�	OneNet_RevPro
//
//	�������ܣ�	ƽ̨�������ݼ��
//
//	��ڲ�����	dataPtr��ƽ̨���ص�����
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void OneNet_RevPro(char *str)
{
	char* strtmp = strtok(str, " ,");
	char* strarr[5]={NULL};
	char index=0;
	char *dataPtr = NULL;
	char numBuf[10];
	int num = 0;
	char *req_payload = NULL;
	char *cmdid_topic = NULL;
	static char buffer[96];
	
	while (strtmp != NULL)
	{
		strarr[index]=strtmp;
		//printf("%s\n", strarr[index]);
		index++;
		strtmp = strtok(NULL, " ,");
		if(NULL == strtmp)
			break;	
	}

	
	req_payload = strarr[4];
	replace_all(strarr[2],"request", "response");
	cmdid_topic = strarr[2];
	
	dataPtr = strchr(req_payload, ':');					//����':'

	if(dataPtr != NULL)					//����ҵ���
	{
		dataPtr++;
		
		while(*dataPtr >= '0' && *dataPtr <= '9')		//�ж��Ƿ����·��������������
		{
			numBuf[num++] = *dataPtr++;
		}
		numBuf[num] = 0;
		
		num = atoi((const char *)numBuf);				//תΪ��ֵ��ʽ
		
		if(strstr((char *)req_payload, "LedB12"))		//����"redled"
		{
			if(num == 1)								//�����������Ϊ1������
			{
				LedB12_Set(LED_ON);
			}
			else if(num == 0)							//�����������Ϊ0�������
			{
				LedB12_Set(LED_OFF);
			}
		}
														//��ͬ
		else if(strstr((char *)req_payload, "LedB13"))
		{
			if(num == 1)
			{
				LedB13_Set(LED_ON);
			}
			else if(num == 0)
			{
				LedB13_Set(LED_OFF);
			}
		}
		else if(strstr((char *)req_payload, "LedB14"))
		{
			if(num == 1)
			{
				LedB14_Set(LED_ON);
			}
			else if(num == 0)
			{
				LedB14_Set(LED_OFF);
			}
		}
		else if(strstr((char *)req_payload, "LedB15"))
		{
			if(num == 1)
			{
				LedB15_Set(LED_ON);
			}
			else if(num == 0)
			{
				LedB15_Set(LED_OFF);
			}
		}
		
		
		sprintf(buffer, "AT+QMTPUBEX=%d,%d,%d,%d,%s,%d\r\n", 0,0,0,0,cmdid_topic,4);
	
		UsartPrintf(USART_DEBUG, "%s",buffer);
		while(ESP8266_SendCmd(buffer,">"))//������ִ
		{
			;
		}
		UsartPrintf(USART_DEBUG, "ojbk\r\n");
		while(ESP8266_SendCmd("ojbk\r\n","OK"))//��ִ��Ϣ
		{
			;
		}
			
	}
	ESP8266_Clear();									//��ջ���
}

//==========================================================
//	�������ƣ�	OneNet_ping
//
//	�������ܣ�	��������
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void OneNet_ping(void)
{
	MQTT_PACKET_STRUCTURE mqtt_packet = {NULL, 0, 0, 0};								//Э���
	
	if(MQTT_PacketPing(&mqtt_packet)==0)
	{
			ESP8266_SendData(mqtt_packet._data, mqtt_packet._len);					//��ƽ̨���Ͷ�������
				
			MQTT_DeleteBuffer(&mqtt_packet);																//ɾ��
	}	

}


void replace_all(char *str, const char *orig, const char *rep) 
{
	// 1. ����һ����̬����buffer���洢�滻����ַ���
	static char buffer[96];
	// 2. ʹ��ָ��pָ��ԭʼ�ַ���str������ȡorig��rep�ĳ��ȡ�
	char *p = str;
	size_t orig_len = strlen(orig);
	size_t rep_len = strlen(rep);
	// 3. ��ѭ�����У�ʹ��strstr��������orig��pָ���λ�ÿ�ʼ֮���һ�γ��ֵ�λ�á�
	while ((p = strstr(p, orig))) {
		// ����ҵ��ˣ��򽫸�λ��֮ǰ�Ĳ��ֿ�����buffer�У�
		strncpy(buffer, str, p - str);
		buffer[p - str] = '\0';
		// ��ʹ��sprintf�������滻������Ӵ���ʣ���ַ���ƴ��������
		sprintf(buffer + (p - str), "%s%s", rep, p + orig_len);
		// ����ٽ����������ԭʼ�ַ����С�
		strcpy(str, buffer);
		// 4.����ָ��p��λ�ã�ʹ��ָ���µ��ַ�������һ����Ҫ�滻���Ӵ�֮���λ�ã�
		// ���ظ���3ֱ���޷����ҵ���Ҫ�滻������Ϊֹ
		p = str + (p - str) + rep_len;
	}
}
