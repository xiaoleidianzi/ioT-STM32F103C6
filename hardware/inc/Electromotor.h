#ifndef _Electromotor_H_
#define _Electromotor_H_

typedef struct
{

	_Bool Electromotor_Enable_Sta;
	_Bool Electromotor_Direction_Sta;
	_Bool Electromotor_Pulse_Sta;

} Electromotor_STATUS;

typedef enum
{

	Electromotor_OFF = 0,
	Electromotor_ON = 1

} Electromotor_ENUM;


void Electromotor_Init(void);

void Electromotor_Enable_Set(Electromotor_ENUM status);

void Electromotor_Direction_Set(Electromotor_ENUM status);

void Electromotor_Pulse_Set(Electromotor_ENUM status);


#endif
