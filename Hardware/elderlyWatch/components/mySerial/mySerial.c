#include "mySerial.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_device.h"

#include "myBT.h"
#include "myFuncs.h"

static const char *TAG = "mySerial";

#define PATTERN_CHR_NUM 3

#define U0_SD_BUF_SIZE (1024)
#define U0_RD_BUF_SIZE (1024)

#define U1_SD_BUF_SIZE (1024)
#define U1_RD_BUF_SIZE (1024)
static QueueHandle_t uart0_queue;
static QueueHandle_t uart1_queue;
#define U0_QUEUE_LEN 32
#define U1_QUEUE_LEN 32

static void uart0_event_task(void *pvParameters)
{
  uart_event_t event;
  size_t buffered_size;
  uint8_t *read_buf = (uint8_t *)malloc(U0_RD_BUF_SIZE+1);
  char bda_str[18] = {0};
  uint8_t bda_uint8[6] = {0};
  int bond_dev_num = 0xff;
  static esp_bd_addr_t bond_dev_list[256];
  esp_err_t ret;
  esp_log_level_set(pcTaskGetName(NULL), ESP_LOG_DEBUG);
  while(1)
  {
    // Waiting for UART event.
    if (xQueueReceive(uart0_queue, (void *)&event, portMAX_DELAY))
    {
      memset(read_buf, 0, U0_RD_BUF_SIZE+1);
      ESP_LOGI(pcTaskGetName(NULL), "uart[0] event:");
      switch (event.type)
      {
      case UART_DATA:
        uart_read_bytes(UART_NUM_0, read_buf, event.size, portMAX_DELAY);
        ESP_LOGI(pcTaskGetName(NULL), "[UART_DATA EVT]: %s", read_buf);
        if(strcmp((char *)read_buf, "-help") == 0){
          ESP_LOGI(pcTaskGetName(NULL), "\n=====menu=====\n-disc-s\t\t开始发现设备\n-disc-c\t\t取消发现设备\n-gbdl\t\t获取已连接设备列表\n-rmbd-[dba]\t从数据库删除指定设备\n-gdn-[bda]\t获取设备[xx:xx:xx:xx:xx:xx]名称\n-send-xxx\t向蓝牙用户端透传xxx数据\n=====menu=====");
        }
        else if(strcmp((char *)read_buf, "-disc-s") == 0){
          esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 0x17, 0);
        }
        else if(strcmp((char *)read_buf, "-disc-c") == 0){
          esp_bt_gap_cancel_discovery();
        }
        else if(strcmp((char *)read_buf, "-gbdl") == 0){
          esp_bt_gap_get_bond_device_list(&bond_dev_num, bond_dev_list);
          ESP_LOGD(pcTaskGetName(NULL), "bond_dev_num=%d", bond_dev_num);
          if(bond_dev_num){
            ESP_LOGI(pcTaskGetName(NULL), "=====Bonded devices=====");
            for(int i = 0; i < bond_dev_num; i++)
              ESP_LOGI(pcTaskGetName(NULL), "%s", bda2str(bond_dev_list[i], bda_str, sizeof(bda_str)));

          }
          else
            ESP_LOGI(pcTaskGetName(NULL), "No Bonded device");
        }
        else if(strncmp((char *)read_buf, "-gdn-", 5) == 0){
          bda_uint8[0] = calc_charTonumber(read_buf[5])*16+calc_charTonumber(read_buf[6]);
          bda_uint8[1] = calc_charTonumber(read_buf[8])*16+calc_charTonumber(read_buf[9]);
          bda_uint8[2] = calc_charTonumber(read_buf[11])*16+calc_charTonumber(read_buf[12]);
          bda_uint8[3] = calc_charTonumber(read_buf[14])*16+calc_charTonumber(read_buf[15]);
          bda_uint8[4] = calc_charTonumber(read_buf[17])*16+calc_charTonumber(read_buf[18]);
          bda_uint8[5] = calc_charTonumber(read_buf[20])*16+calc_charTonumber(read_buf[21]);
          ret = esp_bt_gap_read_remote_name(bda_uint8);
          ESP_LOGD(pcTaskGetName(NULL), "ret=%d", ret);
        }
        else if(strncmp((char *)read_buf, "-send-", 6) == 0){
          if(get_spp_handle() != 0){
            while(get_spp_write_state() != 2){
              vTaskDelay(5/portTICK_PERIOD_MS);
            }
            set_spp_write_state(0);
            esp_spp_write(get_spp_handle(), event.size-6, &read_buf[6]);
          }
        }
        else if(strncmp((char *)read_buf, "-rmbd-", 6) == 0){
          bda_uint8[0] = calc_charTonumber(read_buf[5+1])*16+calc_charTonumber(read_buf[6+1]);
          bda_uint8[1] = calc_charTonumber(read_buf[8+1])*16+calc_charTonumber(read_buf[9+1]);
          bda_uint8[2] = calc_charTonumber(read_buf[11+1])*16+calc_charTonumber(read_buf[12+1]);
          bda_uint8[3] = calc_charTonumber(read_buf[14+1])*16+calc_charTonumber(read_buf[15+1]);
          bda_uint8[4] = calc_charTonumber(read_buf[17+1])*16+calc_charTonumber(read_buf[18+1]);
          bda_uint8[5] = calc_charTonumber(read_buf[20+1])*16+calc_charTonumber(read_buf[21+1]);
          ret = esp_bt_gap_remove_bond_device(bda_uint8);
          ESP_LOGD(pcTaskGetName(NULL), "ret=%d", ret);
        }
        break;
      case UART_BUFFER_FULL:
        uart_flush_input(UART_NUM_0);
        ESP_LOGW(pcTaskGetName(NULL), "[UART_BUFFER_FULL EVT]: Just flush buffer");
        break;
      case UART_FIFO_OVF:
        uart_flush_input(UART_NUM_0);
        ESP_LOGW(pcTaskGetName(NULL), "[UART_FIFO_OVF EVT]: Just flush buffer");
        break;
      // Others
      default:
        ESP_LOGI(pcTaskGetName(NULL), "uart event type: %d", event.type);
        break;
      }
    }
  }
  free(read_buf);
  read_buf = NULL;
  vTaskDelete(NULL);
}

static void uart1_event_task(void *pvParameters)
{
  uart_event_t event;
  size_t buffered_size;
  uint8_t *read_buf = (uint8_t *)malloc(U1_RD_BUF_SIZE+1);
  while(1)
  {
    // Waiting for UART event.
    if (xQueueReceive(uart1_queue, (void *)&event, portMAX_DELAY))
    {
      memset(read_buf, 0, U1_RD_BUF_SIZE+1);
      ESP_LOGI(pcTaskGetName(NULL), "uart[1] event:");
      switch (event.type)
      {
      case UART_DATA:
        uart_read_bytes(UART_NUM_1, read_buf, event.size, portMAX_DELAY);
        ESP_LOGI(pcTaskGetName(NULL), "[UART_DATA EVT]: %s", read_buf);

      // Others
      default:
        ESP_LOGI(pcTaskGetName(NULL), "uart event type: %d", event.type);
        break;
      }
    }
  }
  free(read_buf);
  read_buf = NULL;
  vTaskDelete(NULL);
}

void serial0_init()
{
  // uart 配置参数
  uart_config_t uart_config = {
      .baud_rate = 115200,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_DEFAULT,
  };
  // Install UART driver, and get the queue.
  uart_driver_install(UART_NUM_0, U0_RD_BUF_SIZE, U0_SD_BUF_SIZE, U0_QUEUE_LEN, &uart0_queue, 0);
  uart_param_config(UART_NUM_0, &uart_config);

  // Set UART pins (using UART0 default pins ie no changes.)
  uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

  // 创建 uart 数据处理任务
  xTaskCreate(uart0_event_task, "uart0_event_task", 1024*2, NULL, 12, NULL);
}

void serial1_init()
{
  // uart 配置参数
  uart_config_t uart_config = {
      .baud_rate = 115200,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_DEFAULT,
  };
  // Install UART driver, and get the queue.
  uart_driver_install(UART_NUM_1, U1_RD_BUF_SIZE, U1_SD_BUF_SIZE, U1_QUEUE_LEN, &uart1_queue, 0);
  uart_param_config(UART_NUM_1, &uart_config);

  // Set UART pins
  uart_set_pin(UART_NUM_1, GPIO_NUM_17, GPIO_NUM_16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

  // 创建 uart 数据处理任务
  xTaskCreate(uart1_event_task, "uart1_event_task", 1024*2, NULL, 12, NULL);
}
