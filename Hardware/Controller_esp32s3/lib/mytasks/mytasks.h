#pragma once
#include <Arduino.h>

// 测试用LED相关配置
#define LEDA       GPIO_NUM_10 // IO占用
#define LEDA_OFF   digitalWrite(LEDA , LOW) // 关灯
#define LEDA_ON    digitalWrite(LEDA , HIGH) // 开灯
#define LEDA_PWM   digitalWrite(LEDA , !digitalRead(LEDA )) // 灯闪烁
#define LEDB       GPIO_NUM_11 // IO占用
#define LEDB_OFF   digitalWrite(LEDB , LOW) // 关灯
#define LEDB_ON    digitalWrite(LEDB , HIGH) // 开灯
#define LEDB_PWM   digitalWrite(LEDB , !digitalRead(LEDB )) // 灯闪烁

// 串口1相关配置
#define CUSTOM_FRAME_LEN_MAX 64 // (目前已知Zigbee模块查询状态时返回最多为44字节)

// WiFi
// #define WIFI_SSID "HUAWEI Nova 11 SE"
#define WIFI_SSID "Macbook pro max ultra"
#define WIFI_PASS "roland66"


/************************私协相关定义************************/
typedef struct _customFrame{
  uint8_t *buf;
  uint16_t data_len;
}CustomFrame;

#define REALDATA_LEN_MAX 32
typedef struct _cloudFrame{
  uint8_t short_addr[2]; // 目标终端短地址
  uint8_t device_code; // 目标终端设备类型码
  uint32_t data_len; // 有效数据长度
  uint8_t real_data[REALDATA_LEN_MAX]; // 有效数据
}CloudFrame;



/************************内核数据************************/
// 云端数据帧缓存队列
#define CLOUDFRAME_QUEUE_LEN 32
extern QueueHandle_t cloudFrame_queue;
// 私协数据帧缓存队列
#define CUSTOMFRAME_QUEUE_LEN 100 // 私协队列长度
extern QueueHandle_t customFrame_queue;
// 终端应答标志位
extern SemaphoreHandle_t terminal_ack_semaphore;

//指示用灯光 任务
void LED_task(void *parameter);

// 串口数据处理 任务
void serial_handle_task(void *parameter);

// 终端消息处理任务
void terminal_news_handle_task(void *parameter);

// 云端消息处理任务
void cloud_news_handle_task(void *parameter);

// MQTT数据处理任务
void mqtt_handle_task(void *parameter);
