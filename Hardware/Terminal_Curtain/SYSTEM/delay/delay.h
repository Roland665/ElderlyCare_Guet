#ifndef __DELAY_H
#define __DELAY_H 			   
#include "FreeRTOS.h"
#include "task.h"
 

#define delay(x) vTaskDelay(x / portTICK_PERIOD_MS) // 使用FreeRTOS的ms级延时

void delay_init(void);
void delay_ms(uint16_t xms);
void delay_us(u32 xus);

#endif
