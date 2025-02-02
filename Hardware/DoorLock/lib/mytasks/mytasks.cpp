
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <lvgl.h>
#include "motor_lock.h"
#include "HMACSHA256.h"
#include "mytasks.h"
#include "myFuncs.h"
#include "mymqtt.h"
#include "display.h"
/* 全局变量 */
// RTOS 内核变量
QueueHandle_t input_queue = nullptr; // 输入缓冲队列
QueueHandle_t cloudFrame_queue = nullptr; // 云端数据帧缓存队列
QueueHandle_t serial1Frame_queue = NULL; // 终端私协数据帧队列



/*************************核心1******************************/

/* 外部输入处理 任务 */
/* 输入码定义
0-无输入
1-'1'   2-'2'   3-'3'   4-清空
5-'4'   6-'5'   7-'6'   8-回退
9-'7'   10-'8'  11-'9'  12-关闭
13-'*'  14-'0'  15-'#'  16-确认
*/
void input_handle_task(void *patameter){
  uint8_t input_code; // 外部输入码
  passw_t passw = {
    .password_len = strlen(PASSW_DEFAULT),
    .password = NULL,
    .input_len = 0,
    .input = NULL
  };
  passw.password = (char *)malloc(sizeof(char) * passw.password_len+1);
  strcpy(passw.password, PASSW_DEFAULT);
  passw.input = (char *)malloc(sizeof(char) * 0xFF); // 最多可输入
  passw.input[0] = 0;
  ESP_LOGI("", "The right password is %s, length=%d", passw.password, passw.password_len);
  while (1)
  {
    xQueueReceive(input_queue, &input_code, portMAX_DELAY);
    switch (input_code)
    {
    /* 按下数字 */
    case 1:
      passw.input[passw.input_len++] = '1';
      passw.input[passw.input_len] = 0;
      break;
    case 2:
      passw.input[passw.input_len++] = '2';
      passw.input[passw.input_len] = 0;
      break;
    case 3:
      passw.input[passw.input_len++] = '3';
      passw.input[passw.input_len] = 0;
      break;
    case 5:
      passw.input[passw.input_len++] = '4';
      passw.input[passw.input_len] = 0;
      break;
    case 6:
      passw.input[passw.input_len++] = '5';
      passw.input[passw.input_len] = 0;
      break;
    case 7:
      passw.input[passw.input_len++] = '6';
      passw.input[passw.input_len] = 0;
      break;
    case 9:
      passw.input[passw.input_len++] = '7';
      passw.input[passw.input_len] = 0;
      break;
    case 10:
      passw.input[passw.input_len++] = '8';
      passw.input[passw.input_len] = 0;
      break;
    case 11:
      passw.input[passw.input_len++] = '9';
      passw.input[passw.input_len] = 0;
      break;
    case 14:
      passw.input[passw.input_len++] = '0';
      passw.input[passw.input_len] = 0;
      break;
    /* 按下功能键 */
    case 4: // Clear
      passw.input_len = 0;
      passw.input[passw.input_len] = 0;
      break;
    case 8: // Back
      if(passw.input_len > 0){
        passw.input_len -= 1;
        passw.input[passw.input_len] = 0;
      }
      break;
    case 12: // Close
      passw.input_len = 0;
      passw.input[passw.input_len] = 0;
      locking();
      break;
    case 13: // 结束人脸识别
      Serial1.write(1); // 长度
      Serial1.write(0x00); // 有效数据
      break;
    case 15: // 开始人脸识别
      Serial1.write(1); // 长度
      Serial1.write(0x01); // 有效数据
      break;
    case 16: // Enter
      if(passw.password_len == passw.input_len)
        if(strcmp(passw.password, passw.input) == 0)
          unlocking();
        else{
          ESP_LOGI("", "password error");
          locking();
        }
      else{
        ESP_LOGI("", "password error");
        locking();
      }
      passw.input_len = 0;
      passw.input[passw.input_len] = 0;
      break;
    }
    if(passw.input_len == 0xFF-1){ // 当input数组已被填满
      passw.input_len = 0;
      passw.input[passw.input_len] = 0;
    }
    Serial.println(passw.input);
  }

}


// 串口数据处理 任务
void serial_handle_task(void *parameter){
  uint8_t uart1_buffer[Serial1_FRAME_LEN_MAX];
  uint16_t readLen;
  uint8_t backhome_flag = 0; // 回家标志位，为1表示在屋内，0表示在屋外
  char name[NAME_LEN_MAX];
  while(1){
    readLen = 0;
    while(1){
      // 等待一帧消息接收完毕
      delay(1);
      if(readLen < Serial1.available()){
        // 收到了新消息
        readLen = Serial1.available();
        delay(10); // 等待消息接收完毕
        if(readLen == Serial1.available())
          break; // 若10ms没有收到新字节，则判断一帧消息接收完毕
      }
    }
    // 处理一帧数据
    Serial1.read(uart1_buffer, readLen);

    // 打印康康内容
    Serial.print("Uart1 recived one frame data 0x");
    for(uint16_t i = 0; i < readLen; i++){
      Serial.printf("%02X ", uart1_buffer[i]);
    }
    Serial.println();

    if(uart1_buffer[1] == 00){
      /* 人脸识别失败，关锁 */
      ESP_LOGI("", "recognition failed");
      locking();
      // TODO 在这里写上报消息，人脸识别被触发但不是主人，报警作用
      static char face_result_warn[] = {"{\"doorlock\": {\"server\": {\"command\": \"00\",\"rdata\": \"other\"}}}"};
      IoTDA_client.publish(IOTDA_MSG_UP_TOPIC, face_result_warn);
    }
    else if(uart1_buffer[1] == 0x01){
      /* 人脸识别成功， 开锁并上报被识别人信息 */
      unlocking();
      for(uint8_t i = 2; i < readLen; i++)
        name[i-2] = uart1_buffer[i];
      name[readLen-2] = 0;
      ESP_LOGI("", "Hello, dear %s", name);
      static String face_result_pass;
      face_result_pass.clear();
      face_result_pass = "{\"doorlock\": {\"server\": {\"command\": \"01\",\"rdata\": \"";
      face_result_pass += name;
      face_result_pass += "\"}}}";
      IoTDA_client.publish(IOTDA_MSG_UP_TOPIC, face_result_pass.c_str());
    }
    else if(uart1_buffer[1] == 0x02){
      /* 人脸录入成功 */
      screen->show_progress_InKeyboard(100);
    }
    else if(uart1_buffer[1] == 0x03){
      /* 人脸录入中，反馈录入进度 */
      screen->show_progress_InKeyboard(uart1_buffer[2]);
    }
  }
}


/*************************核心0******************************/

/* 心跳包上报任务 */
#define TEMP_MAX 256
void HB_handle_task(void *parameter){
  DynamicJsonDocument *doc = new DynamicJsonDocument(1024); // 这个值不确定
  char temp[TEMP_MAX];
  while(1){
    // 保障 MQTT 常驻
    if(IoTDA_client.connected()){
      // 更新设备在IotDA的属性
      static JsonArray services;
      static JsonObject properties;
      services = doc->createNestedArray("services");
      services[0]["service_id"] = "default"; // 固定所有产品的属性都属于 “default” 服务
      properties = services[0].createNestedObject("properties");
      if(lock_state)
        properties["lockState"] = "01";
      else
        properties["lockState"] = "00";
      properties["shortAddr"] = "0000"; // 假的短地址
      serializeJson(*doc, temp, measureJson(*doc)+1);
      doc->clear();
      IoTDA_client.publish(IOTDA_RPT_TOPIC, temp);
      ESP_LOGD("", "Report properties:[%d] %s", lock_state, temp);
    }
    else
      Serial.println("IoTDA_client disconnected");
    delay(500);
  }
}

/* 云端消息处理任务 */
void cloud_news_handle_task(void *parameter){
  CloudFrame cFrame;
  uint32_t index;
  BaseType_t ret;

  WiFi.begin(WIFI_SSID, WIFI_PASS); // 尝试连接 wifi
  ESP_LOGI("", "Waiting to link wifi: %s\n", WIFI_SSID);
  // 确保 WiFi 正常连接
  while(WiFi.status() != WL_CONNECTED){
    ESP_LOGI("", "Watting wifi connected...");
    delay(500);
  }
  ESP_LOGD("", "Wifi connected: %s", WIFI_SSID);
  IoTDA_client.setServer(IOTDA_IP, IOTDA_PORT); // 设置连接IoTDA的MQTT服务
  IoTDA_client.setKeepAlive(60);
  IoTDA_client.setCallback(IoTDA_callback); // 设置回调函数入口

  // 获取UTC时间
  time_t time_now; // 存储标准格式时间
  char time_buffer[10+1]; // 存储YYYYMMDDHH的UTC时间戳
  char IotDA_clientId[strlen(IOTDA_DEVICEID)+5+10+1];
  char IotDA_password_encrypted[32*2+1]; // 存放
  configTime(0*3600, 0, "ntp3.aliyun.com");
  do{
    ESP_LOGI(pcTaskGetName(nullptr), "Waiting for time sync...%d", time(nullptr));
    delay(1000);
  }while(time(nullptr) < 1703669609); // 等待时间加载完毕

  // xTaskCreatePinnedToCore(LED_task, "LED_task", 1024*1, NULL, 2, NULL, 0); // 创建任务
  while(1){
    // 保障 MQTT 常驻
    if(!IoTDA_client.loop()){
      time_now = time(nullptr);
      strftime(time_buffer, 11, "%Y%m%d%H", localtime(&time_now)); // 提取关键时间信息
      sprintf(IotDA_clientId, "%s_0_0_%s", IOTDA_DEVICEID, time_buffer); // 合并出客户端ID
      ESP_LOGD(pcTaskGetName(nullptr), "IotDA_clientId is %s", IotDA_clientId);

      // 将 MQTT 连接密码加密
      HMACSHA256_encrypt(IotDA_password_encrypted, (uint8_t *)IOTDA_DEVICESRCRET, (uint8_t *)time_buffer);
      ESP_LOGD(pcTaskGetName(nullptr), "IotDA_password_encrypted is %s", IotDA_password_encrypted);

      // 重连IoTDA服务器
      mqtt_reconnect(IotDA_clientId, IOTDA_DEVICEID, IotDA_password_encrypted, IOTDA_MSG_DOWN_TOPIC);
    }
    else{
      ret = xQueueReceive(cloudFrame_queue, &cFrame, 10/portTICK_RATE_MS); // 等10ms队列数据帧信息
      if(ret != pdTRUE || cFrame.data_len == 0)
        continue;
      index = 0;
      // 打印康康内容
      ESP_LOGI("", "Recived JSON pack from cloud:");
      Serial.print("0x");
      for(int i = 0; i < cFrame.data_len; i++)
        Serial.printf("%02x ", cFrame.real_data[i]);
      Serial.println();
      if(cFrame.real_data[0] == 0x00){
        locking();
      }
      else if(cFrame.real_data[0] == 0x01){
        unlocking();
      }
      else if(cFrame.real_data[0] == 0x02){
        /* 是启动人脸录入命令 */
        Serial1.write(cFrame.data_len); // 有效数据长度
        Serial1.write(cFrame.real_data, cFrame.data_len); // 有效数据
      }
    }
  }
}

//指示用灯光 任务
void LED_task(void *parameter){
  uint8_t count = 0;
  uint8_t inver_flag = 1;
  while(1){
    LEDB_OFF;
    LEDA_ON;
    vTaskDelay(4*count+50);
    LEDB_ON;
    LEDA_OFF;
    vTaskDelay(4*count+50);
    if(inver_flag){
      count++;
      if(count == 50)
        inver_flag = 0;
    }
    else{
      count--;
      if(count == 0)
        inver_flag = 1;
    }
  }
}
