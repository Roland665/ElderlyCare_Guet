#include "delay.h"

#define FREERTOS_USE

#ifdef FREERTOS_USE
#include <FreeRTOS.h>
#include <task.h>

void delay_ms(uint16_t xms){
  vTaskDelay(xms/portTICK_PERIOD_MS);
}

void delay_us(u32 nus)
{
       u32 ticks;
       u32 told,tnow,reload,tcnt=0;
       if((0x0001&(SysTick->CTRL)) ==0)    //��ʱ��δ����
              vPortSetupTimerInterrupt();  //��ʼ����ʱ��

       reload = SysTick->LOAD;                     //��ȡ��װ�ؼĴ���ֵ
       ticks = nus * (SystemCoreClock / 1000000);  //����ʱ��ֵ

       vTaskSuspendAll();//��ֹOS���ȣ���ֹ���us��ʱ
       told=SysTick->VAL;  //��ȡ��ǰ��ֵ�Ĵ���ֵ����ʼʱ��ֵ��
       while(1)
       {
              tnow=SysTick->VAL; //��ȡ��ǰ��ֵ�Ĵ���ֵ
              if(tnow!=told)  //��ǰֵ�����ڿ�ʼֵ˵�����ڼ���
              {
                     if(tnow<told)  //��ǰֵС�ڿ�ʼ��ֵ��˵��δ�Ƶ�0
                          tcnt+=told-tnow; //����ֵ=��ʼֵ-��ǰֵ

                     else     //��ǰֵ���ڿ�ʼ��ֵ��˵���ѼƵ�0�����¼���
                            tcnt+=reload-tnow+told;   //����ֵ=��װ��ֵ-��ǰֵ+��ʼֵ  ��
                                                      //�Ѵӿ�ʼֵ�Ƶ�0��

                     told=tnow;   //���¿�ʼֵ
                     if(tcnt>=ticks)break;  //ʱ�䳬��/����Ҫ�ӳٵ�ʱ��,���˳�.
              }
       }
       xTaskResumeAll();	//�ָ�OS����
}
//SystemCoreClockΪϵͳʱ��(system_stmf4xx.c��)��ͨ��ѡ���ʱ����Ϊ
//systick��ʱ��ʱ�ӣ����ݾ����������

#elif

static u8  fac_us=0;							//us��ʱ������
static uint16_t fac_ms=0;							//ms��ʱ������,

//��ʼ���ӳٺ���
//SYSTICK��ʱ�ӹ̶�ΪHCLKʱ�ӵ�1/8
//SYSCLK:ϵͳʱ��
void delay_init()
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);	//ѡ���ⲿʱ��  HCLK/8
	fac_us=SystemCoreClock/8000000;				//Ϊϵͳʱ�ӵ�1/8

	fac_ms=(uint16_t)fac_us*1000;					//��OS��,����ÿ��ms��Ҫ��systickʱ����

}

//��ʱxus
void delay_us(u32 xus)
{
	u32 temp;
	SysTick->LOAD=xus*fac_us; 					//ʱ�����
	SysTick->VAL=0x00;        					//��ռ�����
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk ;	//��ʼ����
	do
	{
		temp = SysTick->CTRL;
	}while((temp&0x01) && !(temp&(1<<16)));		//�ȴ�ʱ�䵽��
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;	//�رռ�����
	SysTick->VAL =0X00;      					 //��ռ�����
}



//��ʱxms
void delay_ms(uint16_t xms){
	u32 temp;
	while(xms){
		SysTick->LOAD=(u32)fac_ms;				//ʱ�����(SysTick->LOADΪ24bit)
		SysTick->VAL =0x00;							//��ռ�����
		SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//��ʼ����
		do
		{
			temp=SysTick->CTRL;
		}while((temp&0x01)&&!(temp&(1<<16)));		//�ȴ�ʱ�䵽��
		SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//�رռ�����
		SysTick->VAL =0X00;       					//��ռ�����
		xms--;
	}
}

#endif // DEBUG
