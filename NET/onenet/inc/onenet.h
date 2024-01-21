#ifndef _ONENET_H_
#define _ONENET_H_





_Bool OneNET_RegisterDevice(void);

_Bool OneNet_DevLink(void);

void OneNet_SendData(void);

//void OneNET_Subscribe(void);
void OneNET_Subscribe(const char * topic,...);
	
void OneNet_RevPro(unsigned char *cmd);

void OneNet_ping(void);

#endif
