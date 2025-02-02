#ifndef _TIMER_H
#define _TIMER_H
#include "stm32f10x.h"
//PWM控制LED
extern uint16_t PWMval[4];
extern u8 LEDmode[4];
extern u8 direction[4];


//TIM_SetComparex函数指针数组
extern void (*TIM_SetCompare[8])(TIM_TypeDef* TIMx, uint16_t Compare1);


void TIM2_Int_Init(uint16_t arr,uint16_t psc);
void TIM3_PWM_Init(uint16_t arr,uint16_t psc);

void PWM_LED_Light_Set(u8 number, u8 value);
void PWM_LED_Mode(u8 number, u8 mode);
#endif
