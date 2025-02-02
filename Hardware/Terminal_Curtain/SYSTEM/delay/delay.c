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
       if((0x0001&(SysTick->CTRL)) ==0)    //定时器未工作
              vPortSetupTimerInterrupt();  //初始化定时器

       reload = SysTick->LOAD;                     //获取重装载寄存器值
       ticks = nus * (SystemCoreClock / 1000000);  //计数时间值

       vTaskSuspendAll();//阻止OS调度，防止打断us延时
       told=SysTick->VAL;  //获取当前数值寄存器值（开始时数值）
       while(1)
       {
              tnow=SysTick->VAL; //获取当前数值寄存器值
              if(tnow!=told)  //当前值不等于开始值说明已在计数
              {
                     if(tnow<told)  //当前值小于开始数值，说明未计到0
                          tcnt+=told-tnow; //计数值=开始值-当前值

                     else     //当前值大于开始数值，说明已计到0并重新计数
                            tcnt+=reload-tnow+told;   //计数值=重装载值-当前值+开始值  （
                                                      //已从开始值计到0）

                     told=tnow;   //更新开始值
                     if(tcnt>=ticks)break;  //时间超过/等于要延迟的时间,则退出.
              }
       }
       xTaskResumeAll();	//恢复OS调度
}
//SystemCoreClock为系统时钟(system_stmf4xx.c中)，通常选择该时钟作为
//systick定时器时钟，根据具体情况更改

#elif

static u8  fac_us=0;							//us延时倍乘数
static uint16_t fac_ms=0;							//ms延时倍乘数,

//初始化延迟函数
//SYSTICK的时钟固定为HCLK时钟的1/8
//SYSCLK:系统时钟
void delay_init()
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);	//选择外部时钟  HCLK/8
	fac_us=SystemCoreClock/8000000;				//为系统时钟的1/8

	fac_ms=(uint16_t)fac_us*1000;					//非OS下,代表每个ms需要的systick时钟数

}

//延时xus
void delay_us(u32 xus)
{
	u32 temp;
	SysTick->LOAD=xus*fac_us; 					//时间加载
	SysTick->VAL=0x00;        					//清空计数器
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk ;	//开始倒数
	do
	{
		temp = SysTick->CTRL;
	}while((temp&0x01) && !(temp&(1<<16)));		//等待时间到达
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;	//关闭计数器
	SysTick->VAL =0X00;      					 //清空计数器
}



//延时xms
void delay_ms(uint16_t xms){
	u32 temp;
	while(xms){
		SysTick->LOAD=(u32)fac_ms;				//时间加载(SysTick->LOAD为24bit)
		SysTick->VAL =0x00;							//清空计数器
		SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//开始倒数
		do
		{
			temp=SysTick->CTRL;
		}while((temp&0x01)&&!(temp&(1<<16)));		//等待时间到达
		SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//关闭计数器
		SysTick->VAL =0X00;       					//清空计数器
		xms--;
	}
}

#endif // DEBUG
