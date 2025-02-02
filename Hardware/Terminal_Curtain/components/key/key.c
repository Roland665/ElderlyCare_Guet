#include "key.h"

uint8_t net_KeyCD = 0;

uint8_t net_key_time = 0xFF;

// 按键初始化函数
void key_init(){
	net_key_init();
}


// 按键初始化函数
void net_key_init(void) //IO初始化
{
 	GPIO_InitTypeDef GPIO_InitStructure;

 	RCC_APB2PeriphClockCmd(NET_KEY_GPIOClock,ENABLE);//使能NET_KEY时钟

	GPIO_InitStructure.GPIO_Pin  = NET_KEY_Pin;		//设置GPIO_Pin
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 	//设置成上拉输入
 	GPIO_Init(NET_KEY_GPIO, &GPIO_InitStructure);	//初始化KEY_GPIO

}

uint8_t key_scan(void)
{
	if(net_key == 0 && net_KeyCD >= KEYCD_TIME){//如果已经满足了20ms的消抖cd
		net_KeyCD = 0;
		return 0;
	}
 	return 0xFF;// 无按键按下
}
