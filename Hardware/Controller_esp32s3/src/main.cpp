#include <Arduino.h>
#include <queue.h>
#include <PubSubClient.h>
#include <WiFi.h>

#include "mytasks.h"
#include "Zigbee.h"


static const char *TAG = "controler";


void setup() {
  Serial.begin(921600);
  Serial.printf("\n\n\n\n\n\n\n\n\nCPU run in %d MHz\n", ESP.getCpuFreqMHz());
  // 打印PSRAM大小
  Serial.print("Free PSRAM after allocation: ");
  Serial.println(heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
  // LED 任务相关初始化
  pinMode(LEDA, OUTPUT);
  LEDA_ON;
  pinMode(LEDB, OUTPUT);
  // 将 LED 闪烁的任务放在连接WiFi之后

  // MQTT任务相关初始化
  cloudFrame_queue = xQueueCreate(CLOUDFRAME_QUEUE_LEN, sizeof(CloudFrame)); // 创建队列
  xTaskCreatePinnedToCore(cloud_news_handle_task, "cloud_news_handle_task", 1024*10, NULL, 4, NULL, 0);

  // 串口任务相关初始化
  Serial1.begin(115200, SERIAL_8E1, GPIO_NUM_2, GPIO_NUM_1);
  customFrame_queue = xQueueCreate(CUSTOMFRAME_QUEUE_LEN ,sizeof(CustomFrame)); // 创建队列
  terminal_ack_semaphore = xSemaphoreCreateBinary();
  xTaskCreatePinnedToCore(serial_handle_task, "serial_handle_task", 1024*5, NULL, 3, NULL, 1); // 创建任务
  extern Zigbee zigbee;
  // zigbee.start();
  ESP_LOGI("", "Zigbee in UART1 begin~");
  // 终端消息处理任务相关初始化
  xTaskCreatePinnedToCore(terminal_news_handle_task, "terminal_news_handle_task", 1024*3, NULL, 3, NULL, 0); // 创建任务
}

void loop() {

}
