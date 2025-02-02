#pragma once


#include "esp_spp_api.h"

/********configuration********/
#define SPP_SERVER_NAME     "ESP_SPP_SERVER"  /* The name of the SPP server that is displayed to the remote device */
#define BT_DEVICE_NAME      "ESP_Roland"      /* The Bluetooth name displayed to the remote device */
#define BT_DEVICE_ONLY_ONE  1                 /* Whether there can be only one connected device */

typedef struct
{
  uint32_t handle; /* SPP 客户端句柄*/
  uint8_t write_state; /* SPP 消息发送状态 0:发送中 1:发送遇到堵塞 2:发送完毕*/
}spp_dev_t;

typedef struct{
  esp_spp_cb_event_t event; /* 事件枚举 */
  esp_spp_cb_param_t param; /* 事件参数 */
}spp_event_t;


char *bda2str(uint8_t *bda, char *str, size_t size);
void myBT_init(void);
uint32_t get_spp_handle(void);
uint8_t get_spp_write_state(void);
void set_spp_write_state(uint8_t state);
