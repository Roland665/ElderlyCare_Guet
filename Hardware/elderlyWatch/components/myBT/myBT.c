#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/uart.h"

#include "esp_log.h"

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_device.h"

#include "myBT.h"

static const char *TAG = "myBT";

static const esp_spp_sec_t sec_mask = ESP_SPP_SEC_AUTHENTICATE;
static const esp_spp_role_t role_slave = ESP_SPP_ROLE_SLAVE;

#if BT_DEVICE_ONLY_ONE
static spp_dev_t spp_dev={
  .handle = 0,
  .write_state = 2
};
#endif

static QueueHandle_t spp_queue;
#define SPP_QUEUE_LEN 32
#define SPP_QUEUE_SIZE sizeof(spp_event_t)

char *bda2str(uint8_t *bda, char *str, size_t size)
{
  if (bda == NULL || str == NULL || size < 18)
    return NULL;
  uint8_t *p = bda;
  sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
          p[0], p[1], p[2], p[3], p[4], p[5]);
  return str;
}

static bool get_name_from_eir(uint8_t *eir, char *bdname, uint8_t *bdname_len)
{
  uint8_t *rmt_bdname = NULL;
  uint8_t rmt_bdname_len = 0;
  if (!eir || !bdname || !bdname_len) // 传参都需满足非空
    return false;
  rmt_bdname = esp_bt_gap_resolve_eir_data(eir, ESP_BT_EIR_TYPE_CMPL_LOCAL_NAME, &rmt_bdname_len);
  if (!rmt_bdname)
    rmt_bdname = esp_bt_gap_resolve_eir_data(eir, ESP_BT_EIR_TYPE_SHORT_LOCAL_NAME, &rmt_bdname_len);
  if (!rmt_bdname)
    return false;
  if (rmt_bdname_len > ESP_BT_GAP_MAX_BDNAME_LEN) // 设备名称超长了，限长
    rmt_bdname_len = ESP_BT_GAP_MAX_BDNAME_LEN;
  memcpy(bdname, rmt_bdname, rmt_bdname_len);
  bdname[rmt_bdname_len] = '\0';
  *bdname_len = rmt_bdname_len;
  return true;
}

// BT 接入事件回调函数
void esp_bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
  char bda_str[18] = {0};
  int i;
  char disc_bdname[ESP_BT_GAP_MAX_BDNAME_LEN+1];
  uint8_t disc_bdname_len;
  switch (event)
  {
  case ESP_BT_GAP_DISC_RES_EVT:
    ESP_LOGI(TAG, "discover a new device, bda[%s]", bda2str(param->disc_res.bda, bda_str, sizeof(bda_str)));
    for (i = 0; i < param->disc_res.num_prop; i++)
      if (param->disc_res.prop[i].type == ESP_BT_GAP_DEV_PROP_EIR && get_name_from_eir(param->disc_res.prop[i].val, disc_bdname, &disc_bdname_len))
        ESP_LOGI(TAG, "disc_bdname[%s]", disc_bdname);
    break;
  case ESP_BT_GAP_RMT_SRVCS_EVT:
    ESP_LOGI(TAG, "ESP_BT_GAP_RMT_SRVCS_EVT:\nbda:[%s]\nstat:%d\nnum_uuids:%d\nuuid_list:",bda2str(param->rmt_srvcs.bda, bda_str, sizeof(bda_str)),param->rmt_srvcs.stat,param->rmt_srvcs.num_uuids);
    break;
  case ESP_BT_GAP_AUTH_CMPL_EVT: // 认证完成事件
    ESP_LOGI(TAG, "ESP_BT_GAP_AUTH_CMPL_EVT");
    if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS)
      ESP_LOGI(TAG, "authentication success: %s bda:[%s]", param->auth_cmpl.device_name,bda2str(param->auth_cmpl.bda, bda_str, sizeof(bda_str)));
    else
      ESP_LOGE(TAG, "authentication failed, status:%d", param->auth_cmpl.stat);
    break;
  case ESP_BT_GAP_CFM_REQ_EVT: // 简单安全配对事件
    ESP_LOGI(TAG, "ESP_BT_GAP_CFM_REQ_EVT: Please compare the numeric value: %" PRIu32, param->cfm_req.num_val);
    esp_bt_gap_ssp_confirm_reply(param->cfm_req.bda, true);
    break;
  case ESP_BT_GAP_READ_REMOTE_NAME_EVT:
    ESP_LOGI(TAG, "ESP_BT_GAP_READ_REMOTE_NAME_EVT: state[%d]", param->read_rmt_name.stat);
    // if(param->read_rmt_name.stat == 0)
    ESP_LOGI(TAG, "Read BD_Name: [%s]", param->read_rmt_name.rmt_name);
    break;
  case ESP_BT_GAP_MODE_CHG_EVT: // BT mode change
    ESP_LOGI(TAG, "ESP_BT_GAP_MODE_CHG_EVT: mode:%d bda:[%s]", param->mode_chg.mode,bda2str(param->mode_chg.bda, bda_str, sizeof(bda_str)));
    break;
  case ESP_BT_GAP_REMOVE_BOND_DEV_COMPLETE_EVT: // The bonded device was successfully removed
    ESP_LOGI(TAG, "The bonded device [%s] was successfully removed", bda2str(param->remove_bond_dev_cmpl.bda, bda_str, sizeof(bda_str)));
    break;
  case ESP_BT_GAP_ACL_CONN_CMPL_STAT_EVT: // ACL connecttion complete
    ESP_LOGI(TAG, "ESP_BT_GAP_ACL_CONN_CMPL_STAT_EVT: bda:[%s]", bda2str(param->acl_conn_cmpl_stat.bda, bda_str, sizeof(bda_str)));
    break;
  default:
    ESP_LOGI(TAG, "ESP_BT_GAP_XXX_EVT: %d", event);
    break;
  }
  return;
}

// SPP 回调函数
static void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
  spp_event_t spp_event;
  spp_event.event = event;
  memcpy(&spp_event.param, param, sizeof(esp_spp_cb_param_t));
  if(event == ESP_SPP_DATA_IND_EVT){ // 若是数据接收，需单独给接收缓冲区开空间存放
    spp_event.param.data_ind.data = (uint8_t *)malloc(spp_event.param.data_ind.len+1);
    memcpy(spp_event.param.data_ind.data, param->data_ind.data, param->data_ind.len);
  }
  xQueueSend(spp_queue, &spp_event, portMAX_DELAY);
}

// SPP 事件处理任务
static void spp_event_task(void *pvParameters){
  spp_event_t event;
  char bda_str[18] = {0};
  uint8_t *spp_send_buf = (uint8_t *)malloc(ESP_SPP_MAX_MTU+1);
  while(1){
    if(xQueueReceive(spp_queue, &event, portMAX_DELAY)){
      switch (event.event)
      {
      case ESP_SPP_INIT_EVT: /* SPP 初始化事件*/
        if (event.param.init.status == ESP_SPP_SUCCESS)
        {
          ESP_LOGI(pcTaskGetName(NULL), "ESP_SPP_INIT_EVT");
          esp_bt_dev_set_device_name(BT_DEVICE_NAME);
          esp_spp_start_srv(sec_mask, role_slave, 0, SPP_SERVER_NAME);  // Start SPP server
        }
        else
          ESP_LOGE(pcTaskGetName(NULL), "ESP_SPP_INIT_EVT status:%d", event.param.init.status);
        break;
      case ESP_SPP_CLOSE_EVT: /* SPP 连接关闭 */
        ESP_LOGI(pcTaskGetName(NULL), "ESP_SPP_CLOSE_EVT");
#if BT_DEVICE_ONLY_ONE
        esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
        spp_dev.handle = 0;
#endif
        break;
      case ESP_SPP_START_EVT: /* SPP 作为服务器启动 */
        ESP_LOGI(pcTaskGetName(NULL), "ESP_SPP_START_EVT status:%d", event.param.start.status);
        if (event.param.start.status == ESP_SPP_SUCCESS)
        {
          ESP_LOGI(pcTaskGetName(NULL), "ESP_SPP_START_EVT handle:0x%02X sec_id:%d scn:%d", (unsigned int)event.param.start.handle, event.param.start.sec_id,event.param.start.scn);
#if BT_DEVICE_ONLY_ONE
          esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
#endif
        }
        break;
      case ESP_SPP_DATA_IND_EVT: /* 接收数据事件 */
        ESP_LOGI(pcTaskGetName(NULL), "ESP_SPP_DATA_IND_EVT len:%d handle:0x%02X", event.param.data_ind.len, (unsigned int)event.param.data_ind.handle);
        if (event.param.data_ind.len < 128){
          event.param.data_ind.data[event.param.data_ind.len] = 0;
          ESP_LOGI(pcTaskGetName(NULL), "Received: %s", event.param.data_ind.data);
          uart_write_bytes(UART_NUM_1, event.param.data_ind.data, event.param.data_ind.len); /* 无修改直接转发来自APP的消息 */
          free(event.param.data_ind.data);
        }
        else
          ESP_LOGW(pcTaskGetName(NULL), "Rev data longer than 128 byte");
        break;
      case ESP_SPP_CONG_EVT: /* SPP 拥堵状态发生变化 */
        ESP_LOGI(pcTaskGetName(NULL), "ESP_SPP_CONG_EVT");
#if BT_DEVICE_ONLY_ONE
        if(event.param.cong.cong == false)
          spp_dev.write_state = 2;
#endif
        break;
      case ESP_SPP_WRITE_EVT: /* SPP 写操作完成 */
        ESP_LOGI(pcTaskGetName(NULL), "ESP_SPP_WRITE_EVT: cong[%d]", event.param.write.cong);
#if BT_DEVICE_ONLY_ONE
        if(event.param.write.cong == true)
          spp_dev.write_state = 1;
        else
          if(spp_dev.write_state == 0)
            spp_dev.write_state = 2;
#endif
        break;
      case ESP_SPP_SRV_OPEN_EVT: /* SPP 作为服务器连接打开 */
        esp_bt_gap_set_scan_mode(ESP_BT_NON_CONNECTABLE, ESP_BT_NON_DISCOVERABLE);
        ESP_LOGI(pcTaskGetName(NULL), "ESP_SPP_SRV_OPEN_EVT status:%d handle:0x%02X, rem_bda:[%s]", event.param.srv_open.status, (unsigned int)event.param.srv_open.handle, bda2str(event.param.srv_open.rem_bda, bda_str, sizeof(bda_str)));
        strcpy((char *)spp_send_buf, "Hello BT_SPP master, this is "BT_DEVICE_NAME);
#if BT_DEVICE_ONLY_ONE
        spp_dev.handle = event.param.srv_open.handle;
        esp_spp_write(spp_dev.handle, strlen((char *)spp_send_buf), spp_send_buf); /* 发送第一包数据 */
#endif
        break;
      default:
        ESP_LOGI(pcTaskGetName(NULL), "ESP_SPP_XXX_EVT: %d", event.event);
        break;
      }

    }
  }
  free(spp_send_buf);
  vTaskDelete(NULL);
}
// 初始化经典蓝牙
void myBT_init()
{
  /* 创建蓝牙事件处理任务和事件队列 */
  spp_queue = xQueueCreate(SPP_QUEUE_LEN, SPP_QUEUE_SIZE);
  xTaskCreate(spp_event_task, "spp_event_task", 1024*3, NULL, 12, NULL);

  char bda_str[18] = {0};
  // 释放不会使用到的蓝牙模式（BLE）
  ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));
  // 实例化默认的蓝牙配置
  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  // 初始化控制器
  ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
  // 使能控制器
  ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT));
  // 初始化 bluedroid
  ESP_ERROR_CHECK(esp_bluedroid_init());
  // 使能 bluedroid
  ESP_ERROR_CHECK(esp_bluedroid_enable());
  // 注册接入事件回调函数
  ESP_ERROR_CHECK(esp_bt_gap_register_callback(esp_bt_gap_cb));
  // 注册模拟串口(SPP)事件回调函数
  ESP_ERROR_CHECK(esp_spp_register_callback(esp_spp_cb));

  esp_spp_cfg_t bt_spp_cfg = {
      .mode = ESP_SPP_MODE_CB,
      .enable_l2cap_ertm = true,
      .tx_buffer_size = 128,
  };
  ESP_ERROR_CHECK(esp_spp_enhanced_init(&bt_spp_cfg));

#if (CONFIG_BT_SSP_ENABLED == true) /* 看不懂 说是默认安全参数*/
  /* Set default parameters for Secure Simple Pairing */
  esp_bt_sp_param_t param_type = ESP_BT_SP_IOCAP_MODE;
  esp_bt_io_cap_t iocap = ESP_BT_IO_CAP_IO;
  esp_bt_gap_set_security_param(param_type, &iocap, sizeof(uint8_t));
#endif

  /*
   * Set default parameters for Legacy Pairing
   * Use variable pin, input pin code when pairing
   */
  esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_VARIABLE;
  esp_bt_pin_code_t pin_code;
  esp_bt_gap_set_pin(pin_type, 0, pin_code); /* 无 pin 码*/

  ESP_LOGI(TAG, "Bluetooth work successfully, own address:[%s]", bda2str((uint8_t *)esp_bt_dev_get_address(), bda_str, sizeof(bda_str)));
}

#if BT_DEVICE_ONLY_ONE
// 获取SPP客户端设备句柄
uint32_t get_spp_handle(void){
  return spp_dev.handle;
}

// 获取SPP写操作状态
uint8_t get_spp_write_state(void)
{
  return spp_dev.write_state;
}
// 设置SPP写操作状态
void set_spp_write_state(uint8_t state)
{
  spp_dev.write_state = state;
}
#endif
