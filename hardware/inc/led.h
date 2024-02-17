#ifndef _LED_H_
#define _LED_H_





typedef struct
{

	_Bool LedB12Sta;
	_Bool LedB13Sta;
	_Bool LedB14Sta;
	_Bool LedB15Sta;

} LED_STATUS;

extern LED_STATUS led_status;


typedef enum
{

	LED_OFF = 0,
	LED_ON = 1

} LED_ENUM;



void Led_Init(void);

void LedB12_Set(LED_ENUM status);

void LedB13_Set(LED_ENUM status);

void LedB14_Set(LED_ENUM status);

void LedB15_Set(LED_ENUM status);


#endif
