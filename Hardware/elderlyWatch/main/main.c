#include <string.h>

#include "nvs.h"
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "myBT.h"
#include "mySerial.h"
#include "myLED.h"
#include "mywifi.h"
#include "mymqtt.h"

static const char *TAG = "main";

void app_main(void)
{
  esp_log_level_set(TAG, ESP_LOG_VERBOSE);
  esp_err_t ret = nvs_flash_init(); // Init nvs
  // 检查flash是否有无法识别的格式区，若有则擦除并重新 init
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ESP_ERROR_CHECK(nvs_flash_init());
  }
  // LEDS initialize
  LEDS_Init();

  // Serials initialize
  serial0_init();
  serial1_init();

  // WiFi & MQTT initialize
  ESP_ERROR_CHECK(esp_event_loop_create_default()); // 以默认方式创建循环事件组（WiFi 相关事件依赖）
  wifi_init(); // 初始化 WiFi 并自动连接默认的网络
  esp_register_shutdown_handler(&wifi_shutdown); // 注册重启时的回调函数 （防止已重启的设备占用 AP 的网络接口）
  mqtt5_init(); // 配置 MQTT 相关组件

  // Bluetooth initialize
  myBT_init();
}
