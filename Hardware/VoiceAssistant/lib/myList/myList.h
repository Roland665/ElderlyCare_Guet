#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef struct _Terminal_Info_t{
  uint8_t DCode; // 第一位是设备类型码，接着两位是短地址，剩下的是设备名称
  uint8_t SAddr[2];
  char DName[64];
  struct _Terminal_Info_t *next;
}Terminal_Info_t;


typedef char *list_data_t; // 移植时请修改链表内数据所用的数据类型
typedef Terminal_Info_t list_node_t; // 移植时请修改链表所用的数据类型

list_node_t *createList(void);
void headInsertList(list_node_t *list, uint8_t DCode, uint8_t *SAddr,const char *DName);
void headDeleteList(list_node_t *list);
list_node_t *findList(list_node_t *list, const char *DName);
void clearList(list_node_t *list);

// list_node_t *createStack(void);
// void pushStack(list_node_t *s, list_data_t num);
// list_data_t popStack(list_node_t *s);
// list_data_t topStack(list_node_t *s);

// list_node_t *createQueue(void);
// void enqueue(list_node_t *list, list_data_t data);
// list_data_t dequeue(list_node_t *list);
// list_data_t backQueue(list_node_t *list);
// void cleanQueue(list_node_t *list);
// list_data_t readQueueInX(list_node_t *list, int x);

#ifdef __cplusplus
}
#endif
