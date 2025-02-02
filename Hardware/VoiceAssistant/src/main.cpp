
#include <Arduino.h>
#include <driver/gpio.h>
#include <esp_task_wdt.h>
#include <WiFi.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/i2s.h"
#include "driver/gpio.h"
#include "mytasks.h"



static const char* TAG = "Rowling";



void setup(){
  Serial.begin(921600);
  Serial.printf("CPU run in %d MHz\n", ESP.getCpuFreqMHz());
  // 打印PSRAM大小
  Serial.print("Free PSRAM after allocation: ");
  Serial.println(heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
  // 创建消息队列
  OLED_showing_queue = xQueueCreate(1, sizeof(uint8_t)); // OLED显示内容消息队列, 具体内容是uint8数据类型指令
  Wifi_data_queue = xQueueCreate(1, sizeof(char *)); // Wifi账密内容消息队列, 具体内容是char字符串地址

  // 创建各任务
  pinMode(LEDA,OUTPUT);
  pinMode(LEDB,OUTPUT);
  LEDA_ON;

  xTaskCreatePinnedToCore(OLED_task, "OLED_task", 1024*4, NULL, 2, &OLED_Task_Handle, 1);


  xTaskCreatePinnedToCore(serial_handle_task, "serial_handle_task", 1024*5, NULL, 3, NULL, 1); // 创建任务

  pinMode(Chat_Key, INPUT);
  gpio_set_pull_mode(Chat_Key, GPIO_PULLUP_ONLY); // 设置上拉
  xTaskCreatePinnedToCore(chat_task, "chat_task", 1024*8, NULL, 3, NULL, 1);


  // 关闭核心看门狗
  disableCore0WDT();

}


void loop(){

}
