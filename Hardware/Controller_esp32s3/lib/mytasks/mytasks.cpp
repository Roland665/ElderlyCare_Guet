#include <WiFi.h>
#include "HMACSHA256.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "myFuncs.h"
#include "mytasks.h"
#include "Zigbee.h"
#include "mymqtt.h"

Zigbee zigbee(&Serial1); // Zigbee对象
QueueHandle_t customFrame_queue = NULL; // 终端私协数据帧队列
QueueHandle_t cloudFrame_queue = NULL; // 私协数据帧队列
SemaphoreHandle_t terminal_ack_semaphore = NULL; // 终端应答信号量




/*************************核心0******************************/
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


// 终端消息处理任务
#define TEMP_MAX 256
void terminal_news_handle_task(void *parameter){
  CustomFrame cFrame; // 队列数据帧信息(实际内容就是去掉了帧头的数据)
  DynamicJsonDocument *doc = new DynamicJsonDocument(1024); // 这个值不确定
  JsonArray devices;
  JsonArray services;
  JsonObject properties;
  char temp[TEMP_MAX];
  while(1){
    xQueueReceive(customFrame_queue, &cFrame, portMAX_DELAY); // 死等队列数据帧信息(实际内容就是去掉了帧头的数据)

    // // 打印康康内容
    // Serial.print("Handle a custom frame :0x");
    // for(int i = 0; i < cFrame.data_len+10; i++)
    //   Serial.printf("%02X ", cFrame.buf[i]);
    // Serial.println();

    switch (cFrame.buf[8])
    {
    case 0xFE: // 是心跳包
      // 更新对应子设备的在线状态
      static char sub_device_id[16+1] = {0}; // 存放子设备的长地址(字符串)
      static char updateOnlineJson[] = "{\"services\": [{\"service_id\": \"$sub_device_manager\",\"event_type\": \"sub_device_update_status\",\"paras\": {\"device_statuses\": [{\"device_id\":\"1122334455667788\",\"status\":\"ONLINE\"}]}}]}"; // 存放更新子设备状态Json包
      for(uint8_t i = 0; i < 8; i++){
        sub_device_id[2*i] = calc_numberTochar(cFrame.buf[i]/16);
        updateOnlineJson[136+2*i] = sub_device_id[2*i];
        sub_device_id[2*i+1] = calc_numberTochar(cFrame.buf[i]%16);
        updateOnlineJson[136+2*i+1] = sub_device_id[2*i+1];
      }
      IoTDA_client.publish(IOTDA_EVT_UP_TOPIC, updateOnlineJson); // 发布更新信息

      // 更新对应子设备在IotDA的属性
      devices = doc->createNestedArray("devices");
      devices[0]["device_id"] = sub_device_id;
      services = devices[0].createNestedArray("services");
      services[0]["service_id"] = "default"; // 固定所有产品的属性都属于 “default” 服务
      properties = services[0].createNestedObject("properties");
      // 封装子设备短地址
      for(uint8_t i = 0; i < 2; i++){
        temp[2*i] = calc_numberTochar(cFrame.buf[11+i]/16);
        temp[2*i+1] = calc_numberTochar(cFrame.buf[11+i]%16);
      }
      temp[2*2] = 0;
      properties["shortAddr"] = temp;
      switch (cFrame.buf[10])
      {
      case 0x01: // 灯控
        /* 封装灯控亮度状态 */
        for(uint8_t i = 0; i < 4; i++){
          temp[2*i] = calc_numberTochar(cFrame.buf[14+i]/16);
          temp[2*i+1] = calc_numberTochar(cFrame.buf[14+i]%16);
        }
        temp[2*4] = 0;
        properties["brightness"] = temp;
        break;
      case 0x02: // 窗帘
        /* 封装窗帘属性信息 */
        properties["left"] = cFrame.buf[14];
        properties["right"] = cFrame.buf[15];
        properties["humidity"] = cFrame.buf[16]+0.01*cFrame.buf[17];
        properties["temperature"] = cFrame.buf[18]+0.01*cFrame.buf[19];
        break;
      default:
        break;
      }
      serializeJson(*doc, temp, measureJson(*doc)+1);
      doc->clear();
      IoTDA_client.publish(IOTDA_PPT_RPT_TOPIC, temp); // 发布状态更新
      ESP_LOGD("", "Update terminal:[%d], state - ONLINE %s", cFrame.buf[10], temp);
      break;

    default:
      break;
    }
    free(cFrame.buf);
  }
}

// 云端消息处理任务
void cloud_news_handle_task(void *parameter){
  CloudFrame cFrame;
  uint8_t *buffer = NULL;
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

  xTaskCreatePinnedToCore(LED_task, "LED_task", 1024*1, NULL, 2, NULL, 0); // 创建任务
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
      if(ret != pdTRUE)
        continue;
      index = 0;
      // 打印康康内容
      ESP_LOGI("", "Recived JSON pack from cloud:");
      Serial.print("0x");
      Serial.printf("%02X ", cFrame.device_code);
      for(int i = 0; i < cFrame.data_len; i++)
        Serial.printf("%02X ", cFrame.real_data[i]);
      Serial.println();
      if(cFrame.device_code == 0x00){
        if(cFrame.real_data[0] == 0x00){
          zigbee.change_mode(0);
          zigbee.openNet();
          zigbee.change_mode(1);
          ESP_LOGI("", "Open Zigbee net");
        }
      }
      else{ // 对终端控制命令
        // 封装消息缓冲
        buffer = (uint8_t *)malloc(sizeof(uint8_t) * (3+1+1+cFrame.data_len));
        buffer[index++] = '6';
        buffer[index++] = '6';
        buffer[index++] = '5';
        buffer[index++] = cFrame.device_code;
        buffer[index++] = cFrame.data_len;
        while(cFrame.data_len--){
          buffer[index] = cFrame.real_data[index-5];
          index++;
        }

        // 设置透传目标
        zigbee.set_send_target(cFrame.short_addr, 0x01);
        // 进入透传模式
        zigbee.change_mode(1);
        // 发送数据 3s无应答就不管了
        static uint8_t wait_count;
        wait_count = 0;
        do{
          zigbee.send(buffer, index);
          wait_count++;
        }while((xSemaphoreTake(terminal_ack_semaphore, 300) != pdPASS) && wait_count < 10);
        if(wait_count >= 10)
          ESP_LOGI(pcTaskGetName(nullptr), "No terminal ACK");
        else
          ESP_LOGI(pcTaskGetName(nullptr), "Successed sending control to terminal");
      }
      free(buffer);

    }
  }
}


/*************************核心1******************************/

// 串口数据处理 任务
void serial_handle_task(void *parameter){
  uint8_t uart1_buffer[CUSTOM_FRAME_LEN_MAX];
  uint16_t readLen;
  char headFrame[3] = {'6' ,'6', '5'}; // 私协帧头
  CustomFrame cFrame;
  CloudFrame cloud_Frame;
  while(1){
    readLen = 0;
    while(1){
      // 等待一帧消息接收完毕
      delay(1);
      if(readLen < Serial1.available()){
        // 收到了新消息
        readLen = Serial1.available();
        delay(5); // 等待消息接收完毕
        if(readLen == Serial1.available())
          break; // 若5ms没有收到新字节，则判断一帧消息接收完毕
      }
    }
    if(readLen > CUSTOM_FRAME_LEN_MAX){
      /* 接收超长度数据，丢包 */
      ESP_LOGD("", "Receive data too long");
      Serial1.read(uart1_buffer, readLen);
      continue;
    }
    // 先清空缓冲区
    for(uint32_t i = 0; i < CUSTOM_FRAME_LEN_MAX; i++)
      uart1_buffer[i] = 0;
    // 处理一帧数据
    Serial1.read(uart1_buffer, readLen);

    // 打印康康内容
    Serial.print("Uart1 recived one frame data 0x");
    for(uint16_t i = 0; i < readLen; i++)
      Serial.printf("%02X ", uart1_buffer[i]);
    Serial.println();

    if(uart1_buffer[0] == '6' && uart1_buffer[1] == '6' && uart1_buffer[2] == '5'){
      switch(uart1_buffer[11]){
        case 0x00: // 终端请求入网
          // 因为不做用户端处理，则在这里只要收到请求就同意
          cloud_Frame.short_addr[0] = uart1_buffer[14];
          cloud_Frame.short_addr[1] = uart1_buffer[15];
          cloud_Frame.device_code = 0x00;
          cloud_Frame.data_len = 0x01;
          cloud_Frame.real_data[0] = 1; // 直接同意
          xQueueSend(cloudFrame_queue, &cloud_Frame, portMAX_DELAY);
          ESP_LOGI(pcTaskGetName(nullptr), "Accepted terminal:%d`s network application", uart1_buffer[13]);

        case 0xFE: // 终端心跳
          if(uart1_buffer[16] == 1){
            // 将回复终端的数据封装入队列
            cloud_Frame.short_addr[0] = uart1_buffer[14];
            cloud_Frame.short_addr[1] = uart1_buffer[15];
            cloud_Frame.device_code = 0xFF;
            cloud_Frame.data_len = 0x02;
            cloud_Frame.real_data[0] = 'O';
            cloud_Frame.real_data[1] = 'K';
            xQueueSend(cloudFrame_queue, &cloud_Frame, portMAX_DELAY);
            ESP_LOGI(pcTaskGetName(nullptr), "ACK terminal");
          }
          // 再将设备信息数据装入待传云端的队列
          cFrame.buf = (uint8_t *)malloc(sizeof(uint8_t) * (8+1+1+uart1_buffer[12])); // 记得在消息处理任务中释放此空间
          cFrame.data_len = uart1_buffer[12];
          for(uint16_t i = 0; i < cFrame.data_len+10; i++){
            cFrame.buf[i] = uart1_buffer[3+i];
          }
          xQueueSend(customFrame_queue, &cFrame, portMAX_DELAY);
          break;

        case 0xFF: // 应答信息
          xSemaphoreGive(terminal_ack_semaphore);
          ESP_LOGI("", "Recived acknowledge");
          break;
        default:
          // 是私协数据，来自终端，将数据封装入队列
          cFrame.buf = (uint8_t *)malloc(sizeof(uint8_t) * (8+1+1+uart1_buffer[12])); // 记得在消息处理任务中释放此空间
          cFrame.data_len = uart1_buffer[12];
          for(uint16_t i = 0; i < cFrame.data_len+10; i++){
            cFrame.buf[i] = uart1_buffer[3+i];
          }
          xQueueSend(customFrame_queue, &cFrame, portMAX_DELAY);
          ESP_LOGD("", "Successed send one frame data to custom_frame_queue");
          break;
      }
    }
    else if(uart1_buffer[0] == 0x55){
      zigbee.analyze_feedback(uart1_buffer); // 是Zigbee反馈数据
    }
    else if(uart1_buffer[0] == 6 && uart1_buffer[1] == 6 && uart1_buffer[2] == 5){
      // 将语音控制终端的数据封装入队列
      cloud_Frame.short_addr[0] = uart1_buffer[3];
      cloud_Frame.short_addr[1] = uart1_buffer[4];
      cloud_Frame.device_code = uart1_buffer[5];
      cloud_Frame.data_len = uart1_buffer[6];
      for(uint32_t i = 0; i < cFrame.data_len; i++)
        cloud_Frame.real_data[i] = uart1_buffer[7+i];
      xQueueSend(cloudFrame_queue, &cloud_Frame, portMAX_DELAY);
      ESP_LOGI(pcTaskGetName(nullptr), "Voice control successful！");
    }

  // // 测试串口正常工作
  // while(1){
  //   if(Serial1.available()){
  //     Serial1.write(Serial1.read());
  //   }
  // }
  }
}
