#ifndef __USART_H
#define __USART_H
#include "stm32f10x.h"
#include "Zigbee/Zigbee.h"

#define UART_RX_LEN_MAX 200
#define UART1_WAIT_TIME 30

extern uint8_t usart1_rx_buffer[UART_RX_LEN_MAX];//串口接收缓冲区,最大UART_RX_LEN_MAX个字节.
extern uint16_t usart1_rx_len;//接收到的有效字节数目
extern uint8_t usart1RXTime;

void usart1_init(uint32_t bound);
void send_customFrame(Zigbee *zigbee, uint8_t type, uint8_t len, uint8_t* Data);
#endif


