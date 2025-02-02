#ifndef __KEY_H
#define __KEY_H
#include "stm32f10x.h"


//宏定义入网按键的GPIO口
#define NET_KEY_GPIO 		GPIOB
#define NET_KEY_Pin 		GPIO_Pin_12
#define NET_KEY_GPIOClock	RCC_APB2Periph_GPIOB


#define net_key  	GPIO_ReadInputDataBit(NET_KEY_GPIO,NET_KEY_Pin)//读取入网按键

#define KEYCD_TIME 20

extern uint8_t net_KeyCD;

extern uint8_t net_key_time;

void key_init(void);		// IO初始化
void net_key_init(void);
uint8_t key_scan(void);  	// 按键扫描函数
#endif
