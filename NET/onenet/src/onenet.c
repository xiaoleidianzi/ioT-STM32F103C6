/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	onenet.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2017-05-08
	*
	*	版本： 		V1.1
	*
	*	说明： 		与onenet平台的数据交互接口层
	*
	*	修改记录：	V1.0：协议封装、返回判断都在同一个文件，并且不同协议接口不同。
	*				V1.1：提供统一接口供应用层使用，根据不同协议文件来封装协议相关的内容。
	************************************************************
	************************************************************
	************************************************************
**/

//单片机头文件
#include "stm32f10x.h"

//网络设备
#include "esp8266.h"

//协议文件
#include "onenet.h"
#include "mqttkit.h"

//算法
#include "base64.h"
#include "hmac_sha1.h"

//硬件驱动
#include "usart.h"
#include "delay.h"
#include "led.h"

//C库
#include <string.h>
#include <stdio.h>

//加载配置文件
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
*	函数名称：	OTA_UrlEncode
*
*	函数功能：	sign需要进行URL编码
*
*	入口参数：	sign：加密结果
*
*	返回参数：	0-成功	其他-失败
*
*	说明：		+			%2B
*				空格		%20
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
*	函数名称：	OTA_Authorization
*
*	函数功能：	计算Authorization
*
*	入口参数：	ver：参数组版本号，日期格式，目前仅支持格式"2018-10-31"
*				res：产品id
*				et：过期时间，UTC秒值
*				access_key：访问密钥
*				dev_name：设备名
*				authorization_buf：缓存token的指针
*				authorization_buf_len：缓存区长度(字节)
*
*	返回参数：	0-成功	其他-失败
*
*	说明：		当前仅支持sha1
************************************************************
*/
#define METHOD		"sha1"
static unsigned char OneNET_Authorization(char *ver, char *res, unsigned int et, char *access_key, char *dev_name,
											char *authorization_buf, unsigned short authorization_buf_len, _Bool flag)
{
	
	size_t olen = 0;
	
	char sign_buf[64];								//保存签名的Base64编码结果 和 URL编码结果
	char hmac_sha1_buf[64];							//保存签名
	char access_key_base64[64];						//保存access_key的Base64编码结合
	char string_for_signature[72];					//保存string_for_signature，这个是加密的key

//----------------------------------------------------参数合法性--------------------------------------------------------------------
	if(ver == (void *)0 || res == (void *)0 || et < 1564562581 || access_key == (void *)0
		|| authorization_buf == (void *)0 || authorization_buf_len < 120)
		return 1;
	
//----------------------------------------------------将access_key进行Base64解码----------------------------------------------------
	memset(access_key_base64, 0, sizeof(access_key_base64));
	BASE64_Decode((unsigned char *)access_key_base64, sizeof(access_key_base64), &olen, (unsigned char *)access_key, strlen(access_key));
	//UsartPrintf(USART_DEBUG, "access_key_base64: %s\r\n", access_key_base64);
	
//----------------------------------------------------计算string_for_signature-----------------------------------------------------
	memset(string_for_signature, 0, sizeof(string_for_signature));
	if(flag)
		snprintf(string_for_signature, sizeof(string_for_signature), "%d\n%s\nproducts/%s\n%s", et, METHOD, res, ver);
	else
		snprintf(string_for_signature, sizeof(string_for_signature), "%d\n%s\nproducts/%s/devices/%s\n%s", et, METHOD, res, dev_name, ver);
	//UsartPrintf(USART_DEBUG, "string_for_signature: %s\r\n", string_for_signature);
	
//----------------------------------------------------加密-------------------------------------------------------------------------
	memset(hmac_sha1_buf, 0, sizeof(hmac_sha1_buf));
	
	hmac_sha1((unsigned char *)access_key_base64, strlen(access_key_base64),
				(unsigned char *)string_for_signature, strlen(string_for_signature),
				(unsigned char *)hmac_sha1_buf);
	
	//UsartPrintf(USART_DEBUG, "hmac_sha1_buf: %s\r\n", hmac_sha1_buf);
	
//----------------------------------------------------将加密结果进行Base64编码------------------------------------------------------
	olen = 0;
	memset(sign_buf, 0, sizeof(sign_buf));
	BASE64_Encode((unsigned char *)sign_buf, sizeof(sign_buf), &olen, (unsigned char *)hmac_sha1_buf, strlen(hmac_sha1_buf));

//----------------------------------------------------将Base64编码结果进行URL编码---------------------------------------------------
	OTA_UrlEncode(sign_buf);
	//UsartPrintf(USART_DEBUG, "sign_buf: %s\r\n", sign_buf);
	
//----------------------------------------------------计算Token--------------------------------------------------------------------
	if(flag)
		snprintf(authorization_buf, authorization_buf_len, "version=%s&res=products%%2F%s&et=%d&method=%s&sign=%s", ver, res, et, METHOD, sign_buf);
	else
		snprintf(authorization_buf, authorization_buf_len, "version=%s&res=products%%2F%s%%2Fdevices%%2F%s&et=%d&method=%s&sign=%s", ver, res, dev_name, et, METHOD, sign_buf);
	//UsartPrintf(USART_DEBUG, "Token: %s\r\n", authorization_buf);
	
	return 0;

}

//==========================================================
//	函数名称：	OneNET_RegisterDevice
//
//	函数功能：	在产品中注册一个设备
//
//	入口参数：	access_key：访问密钥
//				pro_id：产品ID
//				serial：唯一设备号
//				devid：保存返回的devid
//				key：保存返回的key
//
//	返回参数：	0-成功		1-失败
//
//	说明：		
//==========================================================
_Bool OneNET_RegisterDevice(void)
{

	_Bool result = 1;
	unsigned short send_len = 11 + strlen(DEVICE_NAME);
	char *send_ptr = NULL, *data_ptr = NULL;
	
	char authorization_buf[144];													//加密的key
	
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
	
	data_ptr = (char *)ESP8266_GetIPD(250);							//等待平台响应
	
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
	
	free(send_ptr);//释放内存
	ESP8266_SendCmd("AT+CIPCLOSE\r\n", "OK");
	
	return result;

}

//==========================================================
//	函数名称：	OneNet_DevLink
//
//	函数功能：	与onenet创建连接
//
//	入口参数：	无
//
//	返回参数：	1-失败	0-成功
//
//	说明：		与onenet平台建立连接
//==========================================================
_Bool OneNet_DevLink(void)
{
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};					//协议包

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
		ESP8266_SendData(mqttPacket._data, mqttPacket._len);			//上传平台
		
		dataPtr = ESP8266_GetIPD(250);									//等待平台响应
		if(dataPtr != NULL)
		{
			if(MQTT_UnPacketRecv(dataPtr) == MQTT_PKT_CONNACK)
			{
				switch(MQTT_UnPacketConnectAck(dataPtr))
				{
					case 0:UsartPrintf(USART_DEBUG, "Tips:	连接成功\r\n");status = 0;break;
					
					case 1:UsartPrintf(USART_DEBUG, "WARN:	连接失败：协议错误\r\n");break;
					case 2:UsartPrintf(USART_DEBUG, "WARN:	连接失败：非法的clientid\r\n");break;
					case 3:UsartPrintf(USART_DEBUG, "WARN:	连接失败：服务器失败\r\n");break;
					case 4:UsartPrintf(USART_DEBUG, "WARN:	连接失败：用户名或密码错误\r\n");break;
					case 5:UsartPrintf(USART_DEBUG, "WARN:	连接失败：非法链接(比如token非法)\r\n");break;
					
					default:UsartPrintf(USART_DEBUG, "ERR:	连接失败：未知错误\r\n");break;
				}
			}
		}
		
		MQTT_DeleteBuffer(&mqttPacket);								//删包
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
//	函数名称：	OneNet_SendData
//
//	函数功能：	上传数据到平台
//
//	入口参数：	type：发送数据的格式
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void OneNet_SendData(void)
{
	
	//MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};												//协议包
	
	char buf[256];
	
	short body_len = 0;
	
	UsartPrintf(USART_DEBUG, "Tips:	OneNet_SendData-MQTT\r\n");
	
	memset(buf, 0, sizeof(buf));
	
	body_len = OneNet_FillBuf(buf);																	//获取当前需要发送的数据流的总长度
	
	
	ESP8266_SendData(buf, body_len);
	//ESP8266_SendCmd("AT+QMTSUB=0,1,\"$sys/470285/my_first_device/cmd/#\",0\r\n","OK")//消息订阅
	//if(body_len)
	//{
	//	if(MQTT_PacketSaveData(PROID, DEVICE_NAME, body_len, NULL, &mqttPacket) == 0)				//封包
	//	{
	//		for(; i < body_len; i++)
	//			mqttPacket._data[mqttPacket._len++] = buf[i];
			
	//		ESP8266_SendData(mqttPacket._data, mqttPacket._len);									//上传数据到平台
	//		UsartPrintf(USART_DEBUG, "Send %d Bytes\r\n", mqttPacket._len);
			
	//		MQTT_DeleteBuffer(&mqttPacket);															//删包
	//	}
	//	else
	//		UsartPrintf(USART_DEBUG, "WARN:	EDP_NewBuffer Failed\r\n");
	//}
	
}

//==========================================================
//	函数名称：	OneNET_Publish
//
//	函数功能：	发布消息
//
//	入口参数：	topic：发布的主题
//				msg：消息内容
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void OneNET_Publish(const char *topic, const char *msg)
{

	MQTT_PACKET_STRUCTURE mqtt_packet = {NULL, 0, 0, 0};						//协议包
	
	UsartPrintf(USART_DEBUG, "Publish Topic: %s, Msg: %s\r\n", topic, msg);
	
	if(MQTT_PacketPublish(MQTT_PUBLISH_ID, topic, msg, strlen(msg), MQTT_QOS_LEVEL0, 0, 1, &mqtt_packet) == 0)
	{
		ESP8266_SendData(mqtt_packet._data, mqtt_packet._len);					//向平台发送订阅请求
		
		MQTT_DeleteBuffer(&mqtt_packet);										//删包
	}

}

//==========================================================
//	函数名称：	OneNET_Subscribe
//
//	函数功能：	订阅
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void OneNET_Subscribe(const char * topic,...)
{
	
	MQTT_PACKET_STRUCTURE mqtt_packet = {NULL, 0, 0, 0};						//协议包
	
	char topic_buf[56];
	const char *topic1 = topic_buf;
	va_list ap;
	va_start(ap,topic);
	
	vsnprintf(topic_buf, sizeof(topic_buf),topic,ap);// "$sys/%s/%s/cmd/#", PROID, DEVICE_NAME
	
	va_end(ap);
	
	UsartPrintf(USART_DEBUG, "Subscribe Topic: %s\r\n", topic_buf);
	
	if(MQTT_PacketSubscribe(MQTT_SUBSCRIBE_ID, MQTT_QOS_LEVEL0, &topic1, 1, &mqtt_packet) == 0)
	{
		ESP8266_SendData(mqtt_packet._data, mqtt_packet._len);					//向平台发送订阅请求
		
		MQTT_DeleteBuffer(&mqtt_packet);										//删包
	}

}

////==========================================================
////	函数名称：	OneNET_Subscribe
////
////	函数功能：	订阅
////
////	入口参数：	无
////
////	返回参数：	无
////
////	说明：		
////==========================================================
//void OneNET_Subscribe(void)
//{
//	
//	MQTT_PACKET_STRUCTURE mqtt_packet = {NULL, 0, 0, 0};						//协议包
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
//		ESP8266_SendData(mqtt_packet._data, mqtt_packet._len);					//向平台发送订阅请求
//		
//		MQTT_DeleteBuffer(&mqtt_packet);										//删包
//	}

//}

//==========================================================
//	函数名称：	OneNet_RevPro
//
//	函数功能：	平台返回数据检测
//
//	入口参数：	dataPtr：平台返回的数据
//
//	返回参数：	无
//
//	说明：		
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
	
	dataPtr = strchr(req_payload, ':');					//搜索':'

	if(dataPtr != NULL)					//如果找到了
	{
		dataPtr++;
		
		while(*dataPtr >= '0' && *dataPtr <= '9')		//判断是否是下发的命令控制数据
		{
			numBuf[num++] = *dataPtr++;
		}
		numBuf[num] = 0;
		
		num = atoi((const char *)numBuf);				//转为数值形式
		
		if(strstr((char *)req_payload, "LedB12"))		//搜索"redled"
		{
			if(num == 1)								//控制数据如果为1，代表开
			{
				LedB12_Set(LED_ON);
			}
			else if(num == 0)							//控制数据如果为0，代表关
			{
				LedB12_Set(LED_OFF);
			}
		}
														//下同
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
		while(ESP8266_SendCmd(buffer,">"))//反馈回执
		{
			;
		}
		UsartPrintf(USART_DEBUG, "ojbk\r\n");
		while(ESP8266_SendCmd("ojbk\r\n","OK"))//回执信息
		{
			;
		}
			
	}
	ESP8266_Clear();									//清空缓存
}

//==========================================================
//	函数名称：	OneNet_ping
//
//	函数功能：	心跳请求
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void OneNet_ping(void)
{
	MQTT_PACKET_STRUCTURE mqtt_packet = {NULL, 0, 0, 0};								//协议包
	
	if(MQTT_PacketPing(&mqtt_packet)==0)
	{
			ESP8266_SendData(mqtt_packet._data, mqtt_packet._len);					//向平台发送订阅请求
				
			MQTT_DeleteBuffer(&mqtt_packet);																//删包
	}	

}


void replace_all(char *str, const char *orig, const char *rep) 
{
	// 1. 定义一个静态变量buffer来存储替换后的字符串
	static char buffer[96];
	// 2. 使用指针p指向原始字符串str，并获取orig和rep的长度。
	char *p = str;
	size_t orig_len = strlen(orig);
	size_t rep_len = strlen(rep);
	// 3. 在循环中中，使用strstr函数查找orig在p指向的位置开始之后第一次出现的位置、
	while ((p = strstr(p, orig))) {
		// 如果找到了，则将该位置之前的部分拷贝到buffer中，
		strncpy(buffer, str, p - str);
		buffer[p - str] = '\0';
		// 并使用sprintf函数将替换后的新子串和剩余字符串拼接起来，
		sprintf(buffer + (p - str), "%s%s", rep, p + orig_len);
		// 最后再将结果拷贝回原始字符串中。
		strcpy(str, buffer);
		// 4.更新指针p的位置，使其指向新的字符串中下一个需要替换的子串之后的位置，
		// 并重复步3直到无法再找到需要替换的子由为止
		p = str + (p - str) + rep_len;
	}
}
