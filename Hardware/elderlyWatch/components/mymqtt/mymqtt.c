#include "mymqtt.h"

#include <stdint.h>
#include "esp_log.h"
#include "mqtt_client.h"

#define USE_PROPERTY_ARR_SIZE sizeof(user_property_arr) / sizeof(esp_mqtt5_user_property_item_t)

static const char *TAG = "mymqtt";

// MQTT 用户属性数组
static esp_mqtt5_user_property_item_t user_property_arr[] = {
    {"board", "esp32"},
    {"u", "user"},
    {"p", "password"}};

static esp_mqtt_client_handle_t client = NULL; // 客户端句柄

// 发布主题属性
static esp_mqtt5_publish_property_config_t publish_property = {
    .payload_format_indicator = true, // 是否格式化负载
    .message_expiry_interval = 60,    // 消息过期的时间间隔
    .topic_alias = 0,                 // 主题别名(用整数表示)
    .response_topic = "/topic/test/response",
    .correlation_data = "123456",
    .correlation_data_len = 6,
};

// 订阅属性
static esp_mqtt5_subscribe_property_config_t subscribe_property = {
    .subscribe_id = 25555,
    .no_local_flag = false,            // 服务器是否可以向此主题发布别的客户端的消息
    .retain_as_published_flag = false, // 发布的消息是否作为保留消息
    .retain_handle = 0,                // 是否处理主题的保留消息
    .is_share_subscribe = true,        // 是否为共享订阅
    .share_name = "group1",            // 共享名称
};

// 订阅属性1
static esp_mqtt5_subscribe_property_config_t subscribe1_property = {
    .subscribe_id = 25555,
    .no_local_flag = true,
    .retain_as_published_flag = false,
    .retain_handle = 0,
};

// 取消订阅属性
static esp_mqtt5_unsubscribe_property_config_t unsubscribe_property = {
    .is_share_subscribe = true,
    .share_name = "group1",
};

static esp_mqtt5_disconnect_property_config_t disconnect_property = {
    .session_expiry_interval = 60,
    .disconnect_reason = 0,
};

static void log_error_if_nonzero(const char *message, int error_code)
{
  if (error_code != 0)
  {
    ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
  }
}
// 打印用户属性
static void print_user_property(mqtt5_user_property_handle_t user_property)
{
  if (user_property)
  {
    uint8_t count = esp_mqtt5_client_get_user_property_count(user_property);
    if (count)
    {
      esp_mqtt5_user_property_item_t *item = malloc(count * sizeof(esp_mqtt5_user_property_item_t));
      if (esp_mqtt5_client_get_user_property(user_property, item, &count) == ESP_OK)
      {
        for (int i = 0; i < count; i++)
        {
          esp_mqtt5_user_property_item_t *t = &item[i];
          ESP_LOGI(TAG, "key is %s, value is %s", t->key, t->value);
          free((char *)t->key);
          free((char *)t->value);
        }
      }
      free(item);
    }
  }
}

/**
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args: 用户注册回调函数传入的参数
 * @param base 事件基，在此事件中一直是mqtt_base
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt5_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
  ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32, base, event_id);
  esp_mqtt_event_handle_t event = event_data;
  esp_mqtt_client_handle_t client = event->client;
  int msg_id;
  ESP_LOGD(TAG, "free heap size is %" PRIu32 ", minimum %" PRIu32, esp_get_free_heap_size(), esp_get_minimum_free_heap_size());
  switch ((esp_mqtt_event_id_t)event_id)
  {
  case MQTT_EVENT_CONNECTED: // 连接到服务器事件
    ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
    print_user_property(event->property->user_property); // 打印用户属性

    // 设置发布主题的属性
    esp_mqtt5_client_set_user_property(&publish_property.user_property, user_property_arr, USE_PROPERTY_ARR_SIZE);
    esp_mqtt5_client_set_publish_property(client, &publish_property);
    msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 1);
    esp_mqtt5_client_delete_user_property(publish_property.user_property);
    publish_property.user_property = NULL;
    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

    // 设置订阅主题的属性
    esp_mqtt5_client_set_user_property(&subscribe_property.user_property, user_property_arr, USE_PROPERTY_ARR_SIZE);
    esp_mqtt5_client_set_subscribe_property(client, &subscribe_property);
    msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
    esp_mqtt5_client_delete_user_property(subscribe_property.user_property);
    subscribe_property.user_property = NULL;
    ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

    // 设置订阅主题的属性
    esp_mqtt5_client_set_user_property(&subscribe1_property.user_property, user_property_arr, USE_PROPERTY_ARR_SIZE);
    esp_mqtt5_client_set_subscribe_property(client, &subscribe1_property);
    msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 2);
    esp_mqtt5_client_delete_user_property(subscribe1_property.user_property);
    subscribe1_property.user_property = NULL;
    ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

    esp_mqtt5_client_set_user_property(&unsubscribe_property.user_property, user_property_arr, USE_PROPERTY_ARR_SIZE);
    esp_mqtt5_client_set_unsubscribe_property(client, &unsubscribe_property);
    msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos0");
    ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
    esp_mqtt5_client_delete_user_property(unsubscribe_property.user_property);
    unsubscribe_property.user_property = NULL;
    break;
  case MQTT_EVENT_DISCONNECTED: // 与服务端断开连接事件
    ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
    print_user_property(event->property->user_property);
    break;
  case MQTT_EVENT_SUBSCRIBED: // 订阅成功事件
    ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
    print_user_property(event->property->user_property);
    esp_mqtt5_client_set_publish_property(client, &publish_property);
    msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
    break;
  case MQTT_EVENT_UNSUBSCRIBED: // 取消订阅事件
    ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
    print_user_property(event->property->user_property);
    esp_mqtt5_client_set_user_property(&disconnect_property.user_property, user_property_arr, USE_PROPERTY_ARR_SIZE);
    esp_mqtt5_client_set_disconnect_property(client, &disconnect_property);
    esp_mqtt5_client_delete_user_property(disconnect_property.user_property);
    disconnect_property.user_property = NULL;
    esp_mqtt_client_disconnect(client);
    break;
  case MQTT_EVENT_PUBLISHED: // 发布主题事件
    ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
    print_user_property(event->property->user_property);
    break;
  case MQTT_EVENT_DATA: // 数据事件
    ESP_LOGI(TAG, "MQTT_EVENT_DATA");
    print_user_property(event->property->user_property);
    ESP_LOGI(TAG, "payload_format_indicator is %d", event->property->payload_format_indicator);
    ESP_LOGI(TAG, "response_topic is %.*s", event->property->response_topic_len, event->property->response_topic);
    ESP_LOGI(TAG, "correlation_data is %.*s", event->property->correlation_data_len, event->property->correlation_data);
    ESP_LOGI(TAG, "content_type is %.*s", event->property->content_type_len, event->property->content_type);
    ESP_LOGI(TAG, "TOPIC=%.*s", event->topic_len, event->topic);
    ESP_LOGI(TAG, "DATA=%.*s", event->data_len, event->data);
    break;
  case MQTT_EVENT_ERROR: // 错误事件
    ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
    print_user_property(event->property->user_property);
    ESP_LOGI(TAG, "MQTT5 return code is %d", event->error_handle->connect_return_code);
    if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
    {
      log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
      log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
      log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
      ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
    }
    break;
  default:
    ESP_LOGI(TAG, "MQTT_EVENT_XXX id:[%d]", event->event_id);
    break;
  }
}

// 初始化 MQTT
void mqtt5_init(void)
{
  // 设定 mqtt5 的连接属性
  esp_mqtt5_connection_property_config_t connect_property = {
      .session_expiry_interval = 10,      // 会话过期间隔时间
      .maximum_packet_size = 1024,        // 接受的最大数据包大小
      .receive_maximum = 65535,           // 同时接收的最大数据包数量
      .topic_alias_maximum = 2,           // 支持的最大主题别名
      .request_resp_info = true,          // 是否请求服务器返回响应信息
      .request_problem_info = true,       // 是否在程序执行失败时返回相关信息
      .will_delay_interval = 10,          // 服务器发布遗嘱消息的时间间隔
      .payload_format_indicator = true,   // 是否格式化负载消息
      .message_expiry_interval = 10,      // 消息过期的时间间隔
      .response_topic = "/test/response", // 服务器发生响应所发布的主题(由服务器订阅)
      .correlation_data = "123456",       // 服务器响应数据
      .correlation_data_len = 6,          // 响应数据长度
  };

  // mqtt5 模块配置信息
  esp_mqtt_client_config_t mqtt5_cfg = {
      .broker.address.uri = "mqtt://mqtt.eclipseprojects.io", // 服务器地址
      .session.protocol_ver = MQTT_PROTOCOL_V_5,              // mqtt 协议版本
      .network.disable_auto_reconnect = true,                 // 是否取消自动重连功能
      .credentials.username = "123",                          // 客户端用户名
      .credentials.authentication.password = "456",           // 客户端登入密码
      .session.last_will.topic = "/topic/will",               // 遗嘱发布的主题
      .session.last_will.msg = "i will leave",                // 遗嘱消息
      .session.last_will.msg_len = 12,                        // 遗嘱消息长度
      .session.last_will.qos = 1,                             // 遗嘱消息的 QoS
      .session.last_will.retain = true,                       // 遗嘱消息是否需要服务器保留
  };

  // 初始化 MQTT 客户端
  client = esp_mqtt_client_init(&mqtt5_cfg);

  // 设置用户属性(用于设置连接属性)
  esp_mqtt5_client_set_user_property(&connect_property.user_property, user_property_arr, USE_PROPERTY_ARR_SIZE);
  esp_mqtt5_client_set_user_property(&connect_property.will_user_property, user_property_arr, USE_PROPERTY_ARR_SIZE);
  // 设置连接属性
  esp_mqtt5_client_set_connect_property(client, &connect_property); // 设置完连接属性后可以释放用户属性
  // 释放用户属性
  esp_mqtt5_client_delete_user_property(connect_property.user_property);
  esp_mqtt5_client_delete_user_property(connect_property.will_user_property);

  // 为所有MQTT事件注册服务回调函数
  esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt5_event_handler, NULL); // 为所有MQTT事件注册服务回调函数
  // 启动 MQTT 客户端
  esp_mqtt_client_start(client);
  ESP_LOGI(TAG, "MQTT5 start!");
}
