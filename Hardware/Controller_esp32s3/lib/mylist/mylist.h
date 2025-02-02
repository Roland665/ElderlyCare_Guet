// #pragma once
// #include <Arduino.h>

// //来自终端的数据流链表结点
// typedef struct _terminal_net_info{
//     //数据域
// 	uint8_t LAddr[8]; // 终端长地址
//   uint8_t SAddr[2]; // 终端短地址
//   uint8_t type;     // 终端设备类型码
//   //指针域
//   struct _terminal_net_info* next;
// }terminal_net_info;

// terminal_net_info* create_terminal_net_info_node(uint8_t* LAddr, uint8_t *SAddr, uint8_t type);
// terminal_net_info* create_terminal_net_info_list(void);
// void Insterminal_net_infoNodeByEnd(terminal_net_info* headNode,uint8_t* SLAddr,uint8_t type, uint8_t len, uint8_t* Data);
// void Handleterminal_net_info(terminal_net_info* headNode);
