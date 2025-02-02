#include "led.h"
#include "delay.h"


uint8_t LED1FlashTime = 0;//LED1闪烁剩余时长,归0时停止闪烁

//所有独立IO口控制的LED总的初始函数
void LED_Init(void){
	LED1_Init();
	LED2_Init();

}

void LED1_Init(void){
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(LED1_Clock, ENABLE);	 		//使能端口时钟

	GPIO_InitStructure.GPIO_Pin = LED1_Pin;				//LED1端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//IO口速度为50MHz
	GPIO_Init(LED1_GPIO, &GPIO_InitStructure);			//根据设定参数初始化
	GPIO_SetBits(LED1_GPIO,LED1_Pin);					//LED1输出高
}


void LED2_Init(void){
	GPIO_InitTypeDef  GPIO_InitStructure;
 	RCC_APB2PeriphClockCmd(LED2_Clock, ENABLE);	 		//使能端口时钟

	GPIO_InitStructure.GPIO_Pin = LED2_Pin;	    		//LED2端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//IO口速度为50MHz
	GPIO_Init(LED2_GPIO, &GPIO_InitStructure);	  		//根据设定参数初始化
	GPIO_SetBits(LED2_GPIO,LED2_Pin); 					//LED2 输出高
}


