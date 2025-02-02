#include "timer.h"
#include "delay.h"

//PWM控制LED
uint16_t PWMval[4] = {0};
u8 LEDmode[4] = {0};//LED灯模式，0为普通模式，1为呼吸灯模式
u8 direction[4] = {1,1,1,1};//呼吸灯亮度变化方向，1为增亮，0为减灭
//TIM_SetComparex函数指针数组
void (*TIM_SetCompare[8])(TIM_TypeDef* TIMx, uint16_t Compare1) = {TIM_SetCompare1, TIM_SetCompare2, TIM_SetCompare3, TIM_SetCompare4};

/**
  * @brief    通用定时器2中断初始化
  * @param    arr->自动重装值
  * @param	  psc->时钟预分频数
  * @retval   void
  */

void TIM2_Int_Init(uint16_t arr,uint16_t psc){
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE); ///使能 TIM2 时钟

	//初始化定时器参数
	TIM_TimeBaseStructure.TIM_Prescaler = psc;					//定时器分频
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	//向上计数模式
	TIM_TimeBaseStructure.TIM_Period = arr;						//自动重装载值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		//设置时钟分频因子
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE );//使能TM2更新中断

	//TIM2 中断优先级设置
	NVIC_InitStructure.NVIC_IRQChannel=TIM2_IRQn; 				//定时器 2 中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; 	//抢占优先级 2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x01; 		//响应优先级 3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;				//使能中断
	NVIC_Init(&NVIC_InitStructure);								//初始化 NVIC

	TIM_Cmd(TIM2, ENABLE); //使能 TIM2 外设
}

/**
  * @brief    通用定时器3输出PWM初始化(PA6、PA7、PB0、PB1)
  * @param    arr->自动重装值；psc->时钟预分频数
  * @retval   void
  */
void TIM3_PWM_Init(uint16_t arr,uint16_t psc){
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE); 	//TIM3 时钟使能
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE); 	//使能GPIOA、GPIOB、AFIO时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;	//GPIOA6,A7
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;      	//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//速度50MHz
	GPIO_Init(GPIOA,&GPIO_InitStructure);              		//初始化PA6,PA7

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;	//GPIOB0,B1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;     	//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//速度50MHz
	GPIO_Init(GPIOB,&GPIO_InitStructure);              		//初始化B0,B1

	//初始化TIM3
	TIM_TimeBaseStructure.TIM_Prescaler=psc;  					//定时器分频
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;	//向上计数模式
	TIM_TimeBaseStructure.TIM_Period=arr;   					//自动重装载值
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;		//时钟分隔
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);				//初始化定时器3

	//初始化TIM3 Channel1 PWM模式
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; 				//选择定时器模式:TIM脉冲宽度调制模式
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; 	//比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low ; 		//输出极性:TIM输出比较极性低
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);  						//根据T指定的参数初始化外设TIM3 OC1

	//初始化TIM3 Channel2 PWM模式
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; 				//选择定时器模式:TIM脉冲宽度调制模式
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; 	//比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low ; 		//输出极性:TIM输出比较极性低
	TIM_OC2Init(TIM3, &TIM_OCInitStructure);  						//根据T指定的参数初始化外设TIM3 OC2

	//初始化TIM3 Channel3 PWM模式
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; 				//选择定时器模式:TIM脉冲宽度调制模式
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; 	//比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low ; 		//输出极性:TIM输出比较极性低
	TIM_OC3Init(TIM3, &TIM_OCInitStructure);  						//根据T指定的参数初始化外设TIM3 OC3

	//初始化TIM3 Channel4 PWM模式
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; 				//选择定时器模式:TIM脉冲宽度调制模式
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; 	//比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low ; 		//输出极性:TIM输出比较极性低
	TIM_OC4Init(TIM3, &TIM_OCInitStructure);  						//根据T指定的参数初始化外设TIM3 OC4

	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);//使能TIM3在CCR1上的预装载寄存器
	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);//使能TIM3在CCR2上的预装载寄存器
	TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);//使能TIM3在CCR3上的预装载寄存器
	TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);//使能TIM3在CCR4上的预装载寄存器

	TIM_ARRPreloadConfig(TIM3,ENABLE);//ARPE使能
	TIM_SetCompare1(TIM3,0);
	TIM_SetCompare2(TIM3,0);
	TIM_SetCompare3(TIM3,0);
	TIM_SetCompare4(TIM3,0);
	TIM_Cmd(TIM3, ENABLE);  //使能TIM3
}

// number = 0~4
void PWM_LED_Light_Set(u8 number, u8 value){
  if(number > 4)
    number = 0;

  if(number == 0x00){
    // 同时控制所有灯
    for(uint8_t i = 0; i < 4; i++){
      PWMval[i] = value;
      TIM_SetCompare[i](TIM3, value*5);
    }
  }
  else{
    PWMval[number-1] = value;
    TIM_SetCompare[number-1](TIM3, value*5);
  }
}

// number = 1~4
// mode = 0
//void PWM_LED_Mode(u8 number, u8 mode){
//  if(number > 4)
//    number = 0;

//	if(mode == 0){
//    if
//		while(1){
//			if(PWMval[number]%50 == 0) break;
//			if(direction[number]) PWMval[number]++;
//			else PWMval[number]--;
//            TIM_SetCompare[number](TIM3,PWMval[number]);
//			delay(10);
//		}
//	}
//    LEDmode[number] = mode;
//}
