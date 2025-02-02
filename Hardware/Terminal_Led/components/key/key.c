#include "key.h"

uint8_t net_KeyCD = 0;
uint8_t key1CD = 0;
uint8_t key2CD = 0;
uint8_t key3CD = 0;
uint8_t key4CD = 0;

uint8_t net_key_time = 0xFF;

// ������ʼ������
void key_init(){
	net_key_init();
	key1_init();
	key2_init();
	key3_init();
	key4_init();
}

// ������ʼ������
void key1_init(){
 	GPIO_InitTypeDef GPIO_InitStructure;

 	RCC_APB2PeriphClockCmd(KEY1_GPIOClock,ENABLE);	//ʹ��NET_KEYʱ��

	GPIO_InitStructure.GPIO_Pin  = KEY1_Pin;		//����GPIO_Pin
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 	//���ó���������
 	GPIO_Init(KEY1_GPIO, &GPIO_InitStructure);		//��ʼ��KEY_GPIO
}

// ������ʼ������
void key2_init(){
 	GPIO_InitTypeDef GPIO_InitStructure;

 	RCC_APB2PeriphClockCmd(KEY2_GPIOClock,ENABLE);	//ʹ��NET_KEYʱ��

	GPIO_InitStructure.GPIO_Pin  = KEY2_Pin;		//����GPIO_Pin
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 	//���ó���������
 	GPIO_Init(KEY2_GPIO, &GPIO_InitStructure);		//��ʼ��KEY_GPIO
}

// ������ʼ������
void key3_init(){
 	GPIO_InitTypeDef GPIO_InitStructure;

 	RCC_APB2PeriphClockCmd(KEY3_GPIOClock,ENABLE);	//ʹ��NET_KEYʱ��

	GPIO_InitStructure.GPIO_Pin  = KEY3_Pin;		//����GPIO_Pin
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 	//���ó���������
 	GPIO_Init(KEY3_GPIO, &GPIO_InitStructure);		//��ʼ��KEY_GPIO
}

// ������ʼ������
void key4_init(){
 	GPIO_InitTypeDef GPIO_InitStructure;

 	RCC_APB2PeriphClockCmd(KEY4_GPIOClock,ENABLE);	//ʹ��NET_KEYʱ��

	GPIO_InitStructure.GPIO_Pin  = KEY4_Pin;		//����GPIO_Pin
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 	//���ó���������
 	GPIO_Init(KEY4_GPIO, &GPIO_InitStructure);		//��ʼ��KEY_GPIO
}


// ������ʼ������
void net_key_init(void) //IO��ʼ��
{
 	GPIO_InitTypeDef GPIO_InitStructure;

 	RCC_APB2PeriphClockCmd(NET_KEY_GPIOClock,ENABLE);//ʹ��NET_KEYʱ��

	GPIO_InitStructure.GPIO_Pin  = NET_KEY_Pin;		//����GPIO_Pin
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 	//���ó���������
 	GPIO_Init(NET_KEY_GPIO, &GPIO_InitStructure);	//��ʼ��KEY_GPIO

}

uint8_t key_scan(void)
{
	if(net_key == 0 && net_KeyCD >= KEYCD_TIME){//����Ѿ�������20ms������cd
		net_KeyCD = 0;
		return 0;
	}
	if(key1 == 0 && key1CD >= KEYCD_TIME){//����Ѿ�������20ms������cd
		key1CD = 0;
		return 1;
	}
	if(key2 == 0 && key2CD >= KEYCD_TIME){//����Ѿ�������20ms������cd
		key2CD = 0;
		return 2;
	}
	if(key3 == 0 && key3CD >= KEYCD_TIME){//����Ѿ�������20ms������cd
		key3CD = 0;
		return 3;
	}
	if(key4 == 0 && key4CD >= KEYCD_TIME){//����Ѿ�������20ms������cd
		key4CD = 0;
		return 4;
	}
 	return 0xFF;// �ް�������
}
