#include <Arduino.h>
#include "display.h"
#include "mytasks.h"
#include "motor_lock.h"



void setup()
{
  Serial.begin(921600);
  Serial.printf("\n\n\nCPU run in %d MHz\n", ESP.getCpuFreqMHz());
  /* 打印PSRAM大小 */
  Serial.print("Free PSRAM after allocation: ");
  Serial.println(heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
  lock_init(0.025);
  /* 初始化LVGL */
  screen->init();

  /* FreeRTOS */
  // 处理 MQTT 数据任务
  cloudFrame_queue = xQueueCreate(CLOUDFRAME_QUEUE_LEN, sizeof(CloudFrame)); // 创建队列
  xTaskCreatePinnedToCore(cloud_news_handle_task, "cloud_news_handle_task", 1024*10, NULL, 4, NULL, 0);

  /* 与树莓派串口任务 */
  Serial1.begin(115200, SERIAL_8N1, GPIO_NUM_5, GPIO_NUM_4);
  serial1Frame_queue = xQueueCreate(SERIAL1FRAME_QUEUE_LEN ,sizeof(Serial1Frame)); // 创建队列
  xTaskCreatePinnedToCore(serial_handle_task, "serial_handle_task", 1024*5, NULL, 3, NULL, 1); // 创建任务

  // 处理外部输入任务
  input_queue = xQueueCreate(INPUT_QUEUE_LEN, sizeof(uint8_t)); // 外部输入缓存队列
  xTaskCreatePinnedToCore(input_handle_task, "input_handle_task", 1024*2, nullptr, 3, nullptr, 1);

  // 心跳包上报任务
  xTaskCreatePinnedToCore(HB_handle_task, "HB_handle_task", 1024*5, nullptr, 3, nullptr, 0);

  /* 显示矩阵键盘 */
  screen->show_keyboard();

}

void loop()
{
  screen->run();
  delay(6);
}
