#ifndef _EC800M_H_
#define _EC800M_H_





#define REV_OK		0	//������ɱ�־
#define REV_WAIT	1	//����δ��ɱ�־


void EC800M_Init(void);

void EC800M_Clear(void);

_Bool EC800M_SendCmd(char *cmd, char *res);

void EC800M_SendData(unsigned char *data, unsigned short len);

unsigned char *EC800M_GetIPD(unsigned short timeOut);


#endif
