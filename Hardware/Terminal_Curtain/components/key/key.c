#include "key.h"

uint8_t net_KeyCD = 0;

uint8_t net_key_time = 0xFF;

// ������ʼ������
void key_init(){
	net_key_init();
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
 	return 0xFF;// �ް�������
}
