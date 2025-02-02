#include "myList.h"
#include <stdlib.h>
#include <string.h>

// 创建链表
list_node_t *createList(void){
  list_node_t *list = (list_node_t *)malloc(sizeof(list_node_t));
  list->next = NULL;
  return list;
}

// 头插法插入链表
void headInsertList(list_node_t *list, uint8_t DCode, uint8_t *SAddr,const char *DName){
  list_node_t *newNode = (list_node_t *)malloc(sizeof(list_node_t));
  newNode->DCode = DCode;
  newNode->SAddr[0] = SAddr[0];
  newNode->SAddr[1] = SAddr[1];
  strcpy(newNode->DName, DName);
  newNode->next = list->next;
  list->next = newNode;
}


// 删除链表头部元素
void headDeleteList(list_node_t *list){
  if(list->next == NULL)
    return ;
  list_node_t *temp = list->next;
  list->next = temp->next;
  free(temp);
}

// 查找链表中名称为DName的设备并返回设备数据指针，没找到则返回NULL
list_node_t *findList(list_node_t *list, const char *DName){
  list_node_t *move = list->next;
  while(move){
    if(strcmp(DName, move->DName) == 0)
      return move;
    move = move->next;
  }
  return NULL;
}

// 清空链表
void clearList(list_node_t *list){
  while(list->next)
    headDeleteList(list);
}
