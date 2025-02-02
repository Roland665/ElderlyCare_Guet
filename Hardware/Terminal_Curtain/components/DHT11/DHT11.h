#ifndef __DHT11_H
#define __DHT11_H
#include "delay.h"
#include "stm32f10x.h"

#define IO_DHT11 GPIO_Pin_8
#define GPIO_DHT11 GPIOB

#define DHT11_DQ_High GPIO_SetBits(GPIO_DHT11, IO_DHT11)
#define DHT11_DQ_Low GPIO_ResetBits(GPIO_DHT11, IO_DHT11)

extern uint8_t dht11_data[4];

void DHT11_IO_OUT(void);
void DHT11_IO_IN(void);
void DHT11_Init(void);
uint8_t DHT11_Update_Data(void);
uint8_t DHT11_Read_Byte(void);
uint8_t DHT11_Read_Bit(void);
uint8_t DHT11_Check(void);
void DHT11_Rst(void);

#endif
