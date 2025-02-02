
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "mytasks.h"
#include "myFuncs.h"
#include "mymqtt.h"

WiFiClient wifi_client;                 // 实例化wifi客户端
PubSubClient IoTDA_client(wifi_client);  //  实例化mqtt客户端

// MQTT 接收数据包回调函数(目测发生在核心1的最低优先级任务)
void IoTDA_callback(char *topic, uint8_t *payload, uint32_t length){
  CloudFrame cloud_Frame;
  Serial.print("This topic: ");
  Serial.println(topic);
  Serial.print("recive message:");
  for(int i = 0 ; i < length; i++){
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // 先判断是否为一个正常的 json 包
  if(payload[0] != '{' || payload[length-1] != '}')
    return;
  // 解析JSON
  DynamicJsonDocument doc(length*2); // 声明JSON对象
  deserializeJson(doc, payload);
  String temp;
  if(doc.containsKey("server")){
    if(doc["server"].containsKey("network")){ // 如果是开放入网消息
      // 提取命令码
      temp = doc["server"]["network"]["dcode"].as<String>();
      cloud_Frame.device_code = calc_charTonumber(temp[0])*16+calc_charTonumber(temp[1]);
      // 提取有效数据
      temp = doc["server"]["network"]["rdata"].as<String>();
      cloud_Frame.data_len = (temp.length()+1)/3;
      for(uint32_t i = 0,j=0; i < cloud_Frame.data_len; i++,j++){
        cloud_Frame.real_data[i] = calc_charTonumber(temp[j++])*16+calc_charTonumber(temp[j++]);
      }
    }
    else if(doc["server"].containsKey("control")){ // 如果是对终端控制命令
      temp = doc["server"]["control"]["saddr"].as<String>();
      cloud_Frame.short_addr[0] = calc_charTonumber(temp[0])*16+calc_charTonumber(temp[1]);
      cloud_Frame.short_addr[1] = calc_charTonumber(temp[3])*16+calc_charTonumber(temp[4]);

      temp = doc["server"]["control"]["dcode"].as<String>();
      cloud_Frame.device_code = calc_charTonumber(temp[0])*16+calc_charTonumber(temp[1]);

      temp = doc["server"]["control"]["rdata"].as<String>();
      cloud_Frame.data_len = (temp.length()+1)/3;
      for(uint32_t i = 0,j=0; i < cloud_Frame.data_len; i++,j++){
        cloud_Frame.real_data[i] = calc_charTonumber(temp[j++])*16+calc_charTonumber(temp[j++]);
      }

    }
    // 发送消息
    xQueueSend(cloudFrame_queue, &cloud_Frame, portMAX_DELAY);
    ESP_LOGD("", "Successed send one frame data to cloudFrame_queue");
  }
}

// MQTT 重连函数
void mqtt_reconnect(const char * clientId, const char *userName, const char *password, const char *topic){
  const char *TAG = "mqtt_reconnect";
  while (!IoTDA_client.connected()) {
    ESP_LOGI(TAG, "Attempting MQTT connection...");
    if (IoTDA_client.connect(clientId, userName, password)) {
      ESP_LOGI(TAG, "Connected MQTT, subscribe the topic: %s", topic);
      // 连接成功时订阅主题
      IoTDA_client.subscribe(topic, 1); // 确保消息至少一次发送成功
    } else {
      ESP_LOGW(TAG, "failed to connect the topic: %s, err:%d", topic, IoTDA_client.state());
      ESP_LOGI(TAG, " try again in 5 seconds");
      delay(5000);
    }
  }
}
