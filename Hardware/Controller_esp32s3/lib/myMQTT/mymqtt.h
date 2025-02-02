#pragma once


// IotDA MQTT 相关信息
#define IOTDA_DEVICEID        "6A6B2327004B1200"                                                        // 本设备在IotDA的ID
#define IOTDA_MSG_DOWN_TOPIC  "$oc/devices/"IOTDA_DEVICEID"/sys/messages/down"                          // 订阅的主题，接收平台下发消息
#define IOTDA_MSG_UP_TOPIC    "$oc/devices/"IOTDA_DEVICEID"/sys/messages/up"                            // 发布的主题，向平台发送消息
#define IOTDA_EVT_UP_TOPIC    "$oc/devices/"IOTDA_DEVICEID"/sys/events/up"                              // 发布的主题，向平台更新子设备状态
#define IOTDA_PPT_RPT_TOPIC   "$oc/devices/"IOTDA_DEVICEID"/sys/gateway/sub_devices/properties/report"  // 发布的主题，向平台更新子设备属性 properties report
#define IOTDA_DEVICESRCRET    "c2baa9e2289d30085f541ac7da4bf30f"                                        // 本设备登陆IotDA的密钥
#define IOTDA_IP              "d15fcea36d.st1.iotda-device.cn-north-4.myhuaweicloud.com"                // IotDA服务器的MQTT地址
#define IOTDA_PORT            1883                                                                      // IotDA服务器的MQTT端口号

// MQTT 相关定义(正式)
#define MQTT_SERVER     "10.33.122.78"     // mqtt服务器地址
#define MQTT_PORT       1883                //mqtt服务端口号
#define MQTT_USRNAME    "admin"             //mqtt账户
#define MQTT_PASSWD     "public"            //mqtt密码
#define MQTT_TOPIC      "topic/esp"         //订阅主题
#define MQTT_CLIENT_ID  "controler-client"  //当前设备的clientid标志

// MQTT 相关定义(测试)
#define MQTT_SERVER_DEMO     "10.33.122.78"     // mqtt服务器地址
#define MQTT_PORT_DEMO       1883                //mqtt服务端口号
#define MQTT_USRNAME_DEMO    "admin"             //mqtt账户
#define MQTT_PASSWD_DEMO     "public"            //mqtt密码
#define MQTT_TOPIC_DEMO      "topic/esp"         //订阅主题
#define MQTT_CLIENT_ID_DEMO  "controler-client"  //当前设备的clientid标志

/************************变量声明************************/
extern WiFiClient wifi_client;                 // 实例化wifi客户端
extern PubSubClient IoTDA_client;  //  实例化mqtt客户端

void IoTDA_callback(char *topic, uint8_t *payload, uint32_t length);
void mqtt_reconnect(const char * clientId, const char *userName, const char *password, const char *topic);
