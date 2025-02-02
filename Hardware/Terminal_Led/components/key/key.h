#ifndef __KEY_H
#define __KEY_H
#include "stm32f10x.h"


//宏定义入网按键的GPIO口
#define NET_KEY_GPIO 		GPIOB
#define NET_KEY_Pin 		GPIO_Pin_12
#define NET_KEY_GPIOClock	RCC_APB2Periph_GPIOB

//宏定义按键1的GPIO口
#define KEY1_GPIO 			GPIOA
#define KEY1_Pin 			GPIO_Pin_0
#define KEY1_GPIOClock		RCC_APB2Periph_GPIOA

//宏定义按键2的GPIO口
#define KEY2_GPIO 			GPIOA
#define KEY2_Pin 			GPIO_Pin_1
#define KEY2_GPIOClock		RCC_APB2Periph_GPIOA

//宏定义按键3的GPIO口
#define KEY3_GPIO 			GPIOA
#define KEY3_Pin 			GPIO_Pin_2
#define KEY3_GPIOClock		RCC_APB2Periph_GPIOA

//宏定义按键4的GPIO口
#define KEY4_GPIO 			GPIOA
#define KEY4_Pin 			GPIO_Pin_3
#define KEY4_GPIOClock		RCC_APB2Periph_GPIOA

#define net_key  	GPIO_ReadInputDataBit(NET_KEY_GPIO,NET_KEY_Pin)//读取入网按键
#define key1  		GPIO_ReadInputDataBit(KEY1_GPIO,KEY1_Pin)//读取按键1
#define key2  		GPIO_ReadInputDataBit(KEY2_GPIO,KEY2_Pin)//读取按键2
#define key3  		GPIO_ReadInputDataBit(KEY3_GPIO,KEY3_Pin)//读取按键3
#define key4  		GPIO_ReadInputDataBit(KEY4_GPIO,KEY4_Pin)//读取按键4

#define KEYCD_TIME 20

extern uint8_t net_KeyCD;
extern uint8_t key1CD;
extern uint8_t key2CD;
extern uint8_t key3CD;
extern uint8_t key4CD;

extern uint8_t net_key_time;

void key_init(void);		// IO初始化
void net_key_init(void);
void key1_init(void);
void key2_init(void);
void key3_init(void);
void key4_init(void);
uint8_t key_scan(void);  	// 按键扫描函数
#endif
