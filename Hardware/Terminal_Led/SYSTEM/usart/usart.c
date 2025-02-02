#include "usart.h"
#include "tea/tea.h"
#include <stdlib.h>
#include "stdio.h"
//加入以下代码,支持printf函数,而不需要选择use MicroLIB
#if 1
#pragma import(__use_no_semihosting)
//标准库需要的支持函数
struct __FILE
{
	int handle;

};

FILE __stdout;
//定义_sys_exit()以避免使用半主机模式
void _sys_exit(int x)
{
	x = x;
}
//重定义fputc函数
int fputc(int ch, FILE *f)
{
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕
    USART1->DR = (u8) ch;
	return ch;
}
#endif

u8 usart1_rx_buffer[UART_RX_LEN_MAX] = {0};  //串口接收缓冲区,最大UART_RX_LEN_MAX个字节.
uint16_t usart1_rx_len = 0;//接收到的有效字节数目
u8 usart1RXTime = 0xFF;//串口消息间隔计时，初始化把时间拉满，表示没有收到新的消息

/**
  * @brief		初始化IO 串口1
  * @param		bound->波特率
  * @retval
  */

void usart1_init(u32 bound)
{
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟

	//USART1_TX   GPIOA.9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.9

	//USART1_RX	  GPIOA.10初始化
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.10

	//USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(USART1, &USART_InitStructure); //初始化串口1
	USART_Cmd(USART1, ENABLE);                    //使能串口1

	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器

	USART_ClearFlag(USART1, USART_FLAG_TC);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启串口接收中断
}



/**
  * @brief		将数据封装到私有协议并发送
  * @param		type:自身设备码
  * @param		len :有效数据长度
  * @param		Data :有效数据内容指针（如果是设备信息命令，Data指针指向自身短地址第一位)
  * @retval		void
  */
void send_customFrame(Zigbee *zigbee, u8 type, u8 len, u8* Data){
	u8 i = 0,j = 0,newDataLen = 13+len;
	u8* newData;
//	u8 teaKey[] = {'N','Z','o','k','G','u','z','T','n','F','s','6','D','C','H','4'};
	while(newDataLen % 8 != 0){//把除帧头帧尾数据大小补足到所占内存为8字节的倍数
		newDataLen++;
	}
	newData = (u8*)malloc(newDataLen);
	for(; i < 8; i++){
		newData[i] = zigbee->LAddr[i];
	}
	newData[i] = type;
	i++;
	newData[i] = len;
	i++;
	for(; j<len ;i++,j++){
		newData[i] = Data[j];
	}
	for(; i < newDataLen; i++){
		newData[i] = 1;
	}

	//加密
	// encrypt(newData,newDataLen,teaKey);

	USART_SendData(USART1, '6');
	while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
	USART_SendData(USART1, '6');
	while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
	USART_SendData(USART1, '5');
	while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束

	for(i = 0; i < newDataLen; i++){
		USART_SendData(USART1, newData[i]);
		while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
	}

	free(newData);
}
