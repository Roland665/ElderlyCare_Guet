#ifndef __LED_H_
#define __LED_H_
#include "stm32f10x.h"

#define LED2_GPIO GPIOB
#define LED2_Pin GPIO_Pin_9
#define LED2_Clock RCC_APB2Periph_GPIOB

#define LED1_GPIO GPIOC
#define LED1_Pin GPIO_Pin_13
#define LED1_Clock RCC_APB2Periph_GPIOC


#define set_LED1 GPIO_SetBits(LED1_GPIO, LED1_Pin) // 用于提示终端正在寻找中控
#define reset_LED1 GPIO_ResetBits(LED1_GPIO, LED1_Pin) // 用于提示终端正在寻找中控
#define set_LED2 GPIO_SetBits(LED2_GPIO, LED2_Pin) // 调试直观观察程序进入主while循环
#define reset_LED2 GPIO_ResetBits(LED2_GPIO, LED2_Pin) // 调试直观观察程序进入主while循环

extern uint8_t LED1FlashTime;
void LED_Init(void);//LED GPIO初始化
void LED1_Init(void);
void LED2_Init(void);
#endif
