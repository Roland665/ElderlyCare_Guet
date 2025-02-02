#pragma once

// IotDA 相关信息
#define IOTDA_DEVICEID        "1122334455667788"                                                        // 本设备在IotDA的ID
#define IOTDA_MSG_DOWN_TOPIC  "$oc/devices/"IOTDA_DEVICEID"/sys/messages/down"                          // 订阅的主题，接收平台下发消息
#define IOTDA_MSG_UP_TOPIC    "$oc/devices/"IOTDA_DEVICEID"/sys/messages/up"                            // 发布的主题，向平台发送消息
#define IOTDA_RPT_TOPIC       "$oc/devices/"IOTDA_DEVICEID"/sys/properties/report"  // 发布的主题，向平台更新设备属性
#define IOTDA_DEVICESRCRET    "5358ad66cc416b1531cc03cfcbc65539"                                        // 本设备登陆IotDA的密钥
#define IOTDA_IP              "5f54a81c18.st1.iotda-device.cn-north-4.myhuaweicloud.com"                // IotDA服务器的MQTT地址
#define IOTDA_PORT            1883

/************************变量声明************************/
extern WiFiClient wifi_client;                 // 实例化wifi客户端
extern PubSubClient IoTDA_client;  //  实例化mqtt客户端

void IoTDA_callback(char *topic, uint8_t *payload, uint32_t length);
void mqtt_reconnect(const char * clientId, const char *userName, const char *password, const char *topic);
