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


// WiFi
// #define WIFI_SSID "HUAWEI Nova 11 SE"
#define WIFI_SSID "Macbook pro max ultra"
#define WIFI_PASS "roland66"

// 人脸信息宏定义
#define NAME_LEN_MAX 64 // 名字长度最多为64字节

/************************数据结构定义************************/
#define PASSW_DEFAULT "123456" // 默认门锁密码
typedef struct _passw_t{
  uint8_t password_len; // 正确密码的长度
  char *password; // 正确密码
  uint8_t input_len; // 已输入的密码长度
  char *input; // 已输入的缓冲
}passw_t;

#define REALDATA_LEN_MAX 64+2
typedef struct _CloudFrame{
  uint32_t data_len; // 有效数据长度
  uint8_t real_data[REALDATA_LEN_MAX]; // 有效数据
}CloudFrame;

#define Serial1_FRAME_LEN_MAX 64 // 与树莓派通信时一帧数据的最大值
typedef struct _Serial1Frame{
  uint8_t buf[Serial1_FRAME_LEN_MAX];
  uint16_t data_len;
}Serial1Frame;

/************************内核数据************************/
// 外部输入缓存队列
#define INPUT_QUEUE_LEN 16
extern QueueHandle_t input_queue;
// 云端数据帧缓存队列
#define CLOUDFRAME_QUEUE_LEN 32
extern QueueHandle_t cloudFrame_queue;
// 私协数据帧缓存队列
#define SERIAL1FRAME_QUEUE_LEN 100 // 私协队列长度
extern QueueHandle_t serial1Frame_queue;


// 心跳包上报任务
void HB_handle_task(void *parameter);

// 云端消息处理任务
void cloud_news_handle_task(void *parameter);

// 外部输入处理任务
void input_handle_task(void *parameter);

// 串口数据处理 任务
void serial_handle_task(void *parameter);

// 指示用灯光 任务
void LED_task(void *parameter);


