#ifndef _ONENET_H_
#define _ONENET_H_





_Bool OneNET_RegisterDevice(void);

_Bool OneNet_DevLink(void);

void OneNet_SendData(void);

//void OneNET_Subscribe(void);
void OneNET_Subscribe(const char * topic,...);
	
void OneNet_RevPro(char *cmd);

void OneNet_ping(void);

void replace_all(char *str, const char *orig, const char *rep); 

#endif
