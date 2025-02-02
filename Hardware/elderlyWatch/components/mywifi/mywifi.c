#include "mywifi.h"

#include <string.h>

#include "esp_log.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"

#define STA_NETIF_DESC "sta_netif" // station 的网络接口描述
#define WIFI_CONN_MAX_RETRY 8      // wifi 最大重连次数

static const char *TAG = "mywifi";

char *wifi_ssid = DEFAULT_WIFI_SSID;
char *wifi_pswd = DEFAULT_WIFI_PSWD;

static esp_netif_t *sta_netif_handle = NULL;        // 网络接口句柄
static uint16_t connect_retry_num = 0;              // wifi 重连句柄
static SemaphoreHandle_t get_ip_addrs_semph = NULL; // 等待获取 IP 地址信号量，用于同步事件回调函数与主程序进度
// IP_EVENT_STA_GOT_IP 事件回调函数
static void sta_got_ip_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
  connect_retry_num = 0;
  ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
  // 判断是否为正确的 netif
  ESP_LOGI(TAG, "Into sta_got_ip_handler, event netif desc: %s", esp_netif_get_desc(event->esp_netif));
  if (strncmp(STA_NETIF_DESC, esp_netif_get_desc(event->esp_netif), strlen(STA_NETIF_DESC)) != 0)
    return;

  ESP_LOGI(TAG, "Got IPv4 event: Interface \"%s\" address: " IPSTR, esp_netif_get_desc(event->esp_netif), IP2STR(&event->ip_info.ip));
  if (get_ip_addrs_semph)
  {
    xSemaphoreGive(get_ip_addrs_semph);
  }
  else
  {
    ESP_LOGI(TAG, "- IPv4 address: " IPSTR ",", IP2STR(&event->ip_info.ip));
  }
}

// WIFI_EVENT_STA_DISCONNECTED 事件处理函数
static void sta_disconnect_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
  connect_retry_num++;
  if (connect_retry_num > CONFIG_EXAMPLE_WIFI_CONN_MAX_RETRY)
  {
    ESP_LOGI(TAG, "WiFi Connect failed %d times, stop reconnect.", connect_retry_num);
    /* let example_wifi_sta_do_connect() return */
    if (get_ip_addrs_semph)
    {
      xSemaphoreGive(get_ip_addrs_semph);
    }
    return;
  }
  ESP_LOGI(TAG, "Wi-Fi disconnected, trying to reconnect...");
  esp_err_t err = esp_wifi_connect();
  if (err == ESP_ERR_WIFI_NOT_STARTED)
  {
    return;
  }
  ESP_ERROR_CHECK(err);
}

// 关闭 WiFi
void wifi_shutdown(void)
{

  esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &sta_got_ip_handler);
  esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &sta_disconnect_handler);
  if (get_ip_addrs_semph)
  {
    vSemaphoreDelete(get_ip_addrs_semph);
  }
  esp_wifi_disconnect();

  esp_err_t err = esp_wifi_stop();
  if (err == ESP_ERR_WIFI_NOT_INIT)
  {
    return;
  }
  ESP_ERROR_CHECK(err);
  ESP_ERROR_CHECK(esp_wifi_deinit());
  ESP_ERROR_CHECK(esp_wifi_clear_default_wifi_driver_and_handlers(sta_netif_handle));
  esp_netif_destroy(sta_netif_handle);
  sta_netif_handle = NULL;
}

void wifi_init(void)
{
  get_ip_addrs_semph = xSemaphoreCreateBinary();

  // 初始化 WiFi 模块
  ESP_ERROR_CHECK(esp_netif_init()); // 初始化底层 TCP/IP 协议栈，初始化网络接口
  wifi_init_config_t wifi_init_cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_cfg)); // 初始化 WIFI 相关固件

  esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_WIFI_STA(); // 获取网络接口默认配置
  // Warning: the interface desc is used in tests to capture actual connection details (IP, gw, mask)
  esp_netif_config.if_desc = STA_NETIF_DESC;                              // 设置网络接口的描述
  esp_netif_config.route_prio = 128;                                        // 设置此接口的路由优先级
  sta_netif_handle = esp_netif_create_wifi(WIFI_IF_STA, &esp_netif_config); // 创建 接口对象 对象，为station类型
  esp_wifi_set_default_wifi_sta_handlers();                                 // 设置默认的 station 处理器

  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM)); // 只将 wifi 配置存储在内存中
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));       // 设置WiFi模式为station
  ESP_ERROR_CHECK(esp_wifi_start());                       // 启动 wifi 模块

  // 注册 wifi 和 ip 事件处理函数
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,
                                             IP_EVENT_STA_GOT_IP,
                                             &sta_got_ip_handler,
                                             NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
                                             WIFI_EVENT_STA_DISCONNECTED,
                                             &sta_disconnect_handler,
                                             NULL));

  // 生成 wifi 配置
  wifi_config_t wifi_config = {
      .sta = {
          .scan_method = WIFI_ALL_CHANNEL_SCAN,     // 全通道扫描
          .sort_method = WIFI_CONNECT_AP_BY_SIGNAL, // 按 RSSI 对扫描的 AP 进行排序
          .threshold.rssi = -127,                   // 扫描的 wifi 强度阈值
          .threshold.authmode = WIFI_AUTH_OPEN,     // 开启验证
      },
  };
  memcpy(wifi_config.sta.ssid, wifi_ssid, strlen(wifi_ssid));
  memcpy(wifi_config.sta.password, wifi_pswd, strlen(wifi_pswd));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config)); // 配置 station
  ESP_LOGI(TAG, "Connecting to %s...", wifi_config.sta.ssid);
  esp_err_t ret = esp_wifi_connect();
  if (ret != ESP_OK)
  {
    // 如果 esp_wifi_connect 执行错误，则直接退出链接
    ESP_LOGE(TAG, "WiFi connect failed! ret:%02X", ret);
    return;
  }

  // 等待获取 AP 的 IP 或多次尝试连接失败
  ESP_LOGI(TAG, "Waiting for IP(s)");
  xSemaphoreTake(get_ip_addrs_semph, portMAX_DELAY);
  if (connect_retry_num > WIFI_CONN_MAX_RETRY)
  {
    ESP_LOGE(TAG, "WiFi connect failed! Try connecting too much times");
    return;
  }

  // 打印 station 的 IP 信息
  esp_netif_t *netif = NULL;
  while ((netif = esp_netif_next_unsafe(netif)) != NULL)
  {
    ESP_LOGI(TAG, "  ");
    if (strncmp(esp_netif_get_desc(netif), STA_NETIF_DESC, strlen(STA_NETIF_DESC)) == 0)
    {
      ESP_LOGI(TAG, "Connected to %s", esp_netif_get_desc(netif));

      esp_netif_ip_info_t ip;
      ESP_ERROR_CHECK(esp_netif_get_ip_info(netif, &ip));

      ESP_LOGI(TAG, "- IPv4 address: " IPSTR ",", IP2STR(&ip.ip));
    }
  }
}

// esp_err_t wifi_connect(void){

// }
