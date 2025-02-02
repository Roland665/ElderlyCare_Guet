#include "led.h"
#include "delay.h"


uint8_t LED1FlashTime = 0;//LED1��˸ʣ��ʱ��,��0ʱֹͣ��˸

//���ж���IO�ڿ��Ƶ�LED�ܵĳ�ʼ����
void LED_Init(void){
	LED1_Init();
	LED2_Init();

}

void LED1_Init(void){
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(LED1_Clock, ENABLE);	 		//ʹ�ܶ˿�ʱ��

	GPIO_InitStructure.GPIO_Pin = LED1_Pin;				//LED1�˿�����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//IO���ٶ�Ϊ50MHz
	GPIO_Init(LED1_GPIO, &GPIO_InitStructure);			//�����趨������ʼ��
	GPIO_SetBits(LED1_GPIO,LED1_Pin);					//LED1�����
}


void LED2_Init(void){
	GPIO_InitTypeDef  GPIO_InitStructure;
 	RCC_APB2PeriphClockCmd(LED2_Clock, ENABLE);	 		//ʹ�ܶ˿�ʱ��

	GPIO_InitStructure.GPIO_Pin = LED2_Pin;	    		//LED2�˿�����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//IO���ٶ�Ϊ50MHz
	GPIO_Init(LED2_GPIO, &GPIO_InitStructure);	  		//�����趨������ʼ��
	GPIO_SetBits(LED2_GPIO,LED2_Pin); 					//LED2 �����
}


