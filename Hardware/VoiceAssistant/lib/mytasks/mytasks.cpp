#include "mytasks.h"
#include "API_online.h"
#include "ERNIE-Bot.h"
#include "sd_card.h"
#include "U8g2lib.h"
#include "WavFileReader.h"
#include "WavFileWriter.h"
#include "I2SINMPSampler.h"
#include "myList.h"
#include <myserver.h>

/* UI指令内容
0-显示待机内容
1-显示录音中UI
2-显示识别中UI
3-显示语音识别结果
4-组件准备中
5-没网了
6-语音识别的录音时长按时间过短
7-网络质量不佳，重新操作
8-内存不足，malloc失败
9-语音识别内容为空文本-你说话了O.o?
10-刚启动时显示尝试连接的wifi名
11-开始设置WIFI，请输入wifi名
12-请输入wifi密码
*/

// 驱动INMP441的I2S配置
const i2s_config_t i2s_INMP_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX), // 接收模式
    .sample_rate = SAMPLE_RATE,                          // 采样率
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,        // 样本位深
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,         // leave L/R unconnected when using Left channel
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 64, // DMA缓冲区数量
    .dma_buf_len = 8,    // DMA缓冲区长度
    .use_apll = 1        // use APLL-CLK,frequency 16MHZ-128MHZ,it's for audio
};

// 驱动INMP441的I2S引脚
const i2s_pin_config_t i2s_INMP_pin_config = {
    .bck_io_num = MIC_INMP_I2S_BCK,
    .ws_io_num = MIC_INMP_I2S_WS,
    .data_out_num = MIC_INMP_I2S_DATA_OUT,
    .data_in_num = MIC_INMP_I2S_DATA_IN};

// 驱动I2S扬声器模块的配置
i2s_config_t i2s_player_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX), // 发送模式
    .sample_rate = 0,                                    // 采样率
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,        // 样本位深
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,        // leave L/R unconnected when using Left channel
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_MSB),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 16, // DMA缓冲区数量
    .dma_buf_len = 1024, // DMA缓冲区长度
                         // .use_apll = 1                                       // use APLL-CLK,frequency 16MHZ-128MHZ,it's for audio
};

// 驱动INMP441的I2S引脚
const i2s_pin_config_t i2s_player_pin_config = {
    .bck_io_num = PLAYER_I2S_BCK,
    .ws_io_num = PLAYER_I2S_WS,
    .data_out_num = PLAYER_I2S_DATA_OUT,
    .data_in_num = PLAYER_I2S_DATA_IN};

// MicroSD card SPI 总线配置信息
spi_bus_config_t sd_spi_bus_config = {
    .mosi_io_num = SD_SPI_MOSI,
    .miso_io_num = SD_SPI_MISO,
    .sclk_io_num = SD_SPI_SCLK,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz = 4000,
};

static char *wifi_ssid = "Macbook pro max ultra";
static char *wifi_password = "roland66";
// static char *wifi_ssid = "Xiaomi_4c";
// static char *wifi_password = "l18005973661";
bool recordFlag = false; // 是否开始录音标志位，true-开始
bool LEDT_flag = false;
hw_timer_t *Timer0 = NULL;                                            // 预先定义一个指针来存放定时器的位置
String sis_payload;                                                   // 语音识别后的文本
API_online sis_api("cn-north-4", "9512b326c85747cbade191a38a51691c"); // 设置云服务器结点、项目ID
ERNIE_API bot_api;
API_Server *server_api = new API_Server();
String chat_content;
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE, /* clock=*/OLED_SCL, /* data=*/OLED_SDA);
sd_card *rowl_sd; // 挂载的TF卡对象
Zigbee *zigbee = new Zigbee(&Serial1);

QueueHandle_t OLED_showing_queue = NULL; // OLED显示内容消息队列, 具体内容是uint8数据类型指令
QueueHandle_t Wifi_data_queue = NULL;    // Wifi账密内容消息队列, 具体内容是char字符串地址

/***任务句柄***/
TaskHandle_t OLED_Task_Handle;

/*************************核心1******************************/

// 串口数据处理 任务
#define CUSTOM_FRAME_LEN_MAX 50 // (目前已知Zigbee模块查询状态时返回最多为44字节)
void serial_handle_task(void *parameter){
  uint8_t uart1_buffer[CUSTOM_FRAME_LEN_MAX];
  uint16_t readLen;
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
      Serial.printf("%x ", uart1_buffer[i]);
    }
    Serial.println();
    if(uart1_buffer[0] == 0x55){
      zigbee->analyze_feedback(uart1_buffer); // 是Zigbee反馈数据
    }
  }
}

// chat_task 任务函数
void chat_task(void *parameter)
{
  uint8_t queue_temp;      //
  int bytes_read = 0xffff; // 单次实际采样数
  int16_t err;

  const char *wavPath = "voice.wav"; // 录音文件路径
  // 挂载MicroSD Card
  rowl_sd = new sd_card(SD_MOUNT_POINT, sd_spi_bus_config, SD_SPI_CS); // 实例化TF卡读写对象

  WavFileWriter mic_WavFileWriter(rowl_sd, wavPath, SAMPLE_RATE, sizeof(i2s_INMP_sample_t)); // 实例化一个wav文件写手 对象

  I2SINMPSampler mic_I2SINMPSampler(MIC_INMP_I2SPORT, i2s_INMP_config, i2s_INMP_pin_config); // 实例化一个INMP441的采样器 对象

  i2s_INMP_sample_t *samples_read = (i2s_INMP_sample_t *)malloc(sizeof(i2s_INMP_sample_t) * i2s_INMP_config.dma_buf_len); // 开空间存储采样数据

  /* 初始化 Zigbee */
  zigbee->start();

  // 连接wifi并获取sis_api token
  ESP_LOGI(pcTaskGetName(NULL), "Default wifi: %s", wifi_ssid);
  // 先尝试连接默认wifi,等待蓝牙写入新的wifi账密
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED)
  {
    ESP_LOGI(pcTaskGetName(NULL), "Waiting to link wifi: %s", wifi_ssid);
    vTaskDelay(500);
    queue_temp = 10;
    xQueueSend(OLED_showing_queue, &queue_temp, portMAX_DELAY); // 告诉OLED显示待联网UI
    break;
  }
  ESP_LOGI(pcTaskGetName(NULL), "Succeed in linking wifi: %s", wifi_ssid);

  // server_api->get_terminal_list();
  sis_api.start(); // 获取token
  bot_api.start();
  sis_api.queryHotList(); // 查询云端并更新本地热词表

  // 进入待机模式
  queue_temp = 0;
  xQueueSend(OLED_showing_queue, &queue_temp, portMAX_DELAY);
  xTaskCreatePinnedToCore(LED_task, "LED_task", 1024*2, NULL, 1, NULL, 0); // 启用LED闪烁
  while (1)
  {
    if (digitalRead(Chat_Key) == 0)
    {
      recordFlag = true;
      vTaskDelay(10);
      ESP_LOGI(pcTaskGetName(NULL), "Record begin~");
      queue_temp = 1;
      // 启用i2s模块获取INMP441音频数据
      mic_I2SINMPSampler.start();
      // 开始向目标文件写入wav头片段
      mic_WavFileWriter.start();
      // vTaskDelay(500);
      xQueueSend(OLED_showing_queue, &queue_temp, portMAX_DELAY); // 告诉OLED显示“录音中”UI
      vTaskDelay(100);
      vTaskSuspend(OLED_Task_Handle); // 挂起用到IIC的任务，影响到spi写入SD卡了
      while (digitalRead(Chat_Key) == 0)
      {
        // 采样录音
        bytes_read = mic_I2SINMPSampler.read(samples_read, i2s_INMP_config.dma_buf_len);
        for (int i = 0; i < bytes_read / sizeof(i2s_INMP_sample_t); i++)
        {
          samples_read[i] *= 11;
          if (samples_read[i] > 32767)
            samples_read[i] = 32767;
          else if (samples_read[i] < -32768)
            samples_read[i] = -32768;
        }
        mic_WavFileWriter.write((uint8_t *)samples_read, sizeof(i2s_INMP_sample_t), bytes_read);
      }
      recordFlag = false;
      ESP_LOGI(pcTaskGetName(NULL), "Record stop~");
      vTaskResume(OLED_Task_Handle);
      // 结束I2S采样
      mic_I2SINMPSampler.stop();
      // 结束wav文件的写入
      mic_WavFileWriter.finish();
      queue_temp = 2;
      xQueueSend(OLED_showing_queue, &queue_temp, portMAX_DELAY); // 告诉OLED显示识别中UI
      // 对wav文件进行base64编码
      char *base64_data = rowl_sd->enBase64(wavPath);
      // 将wav文件的base64编码POST到SIS，接收返回数据
    sis_api_again:
      err = sis_api.sis(base64_data, &sis_payload);
      if(err == 0)
        server_api->analyse_command(sis_payload.c_str(), zigbee);
      ESP_LOGD(pcTaskGetName(NULL), "err=%d", err);
      switch (err)
      {
      case -2:              // 与服务器的连接断开了
        goto sis_api_again; // 重新来一次
        break;
      case 0: // 成功识别
              // 将语音转文本的数据传给模型
      bot_api_again:
        err = bot_api.chat(sis_payload, &chat_content);
        ESP_LOGD(pcTaskGetName(NULL), "err=%d", err);
        switch (err)
        {
        case -2:              // 与服务器的连接断开了
          goto bot_api_again; // 重新来一次
          break;
        case 0: // 成功聊天
          queue_temp = 3;
          xQueueSend(OLED_showing_queue, &queue_temp, portMAX_DELAY); // 告诉OLED显示语音识别结果UI
          break;
        case 1: // 没网了
          queue_temp = 5;
          xQueueSend(OLED_showing_queue, &queue_temp, portMAX_DELAY);
          break;
        case 2: // 网络连接不畅
          queue_temp = 7;
          xQueueSend(OLED_showing_queue, &queue_temp, portMAX_DELAY);
          break;
        }
        break;
      case 1: // 没网了
        queue_temp = 5;
        xQueueSend(OLED_showing_queue, &queue_temp, portMAX_DELAY);
        break;
      case 2: // 内存不足了
        queue_temp = 8;
        xQueueSend(OLED_showing_queue, &queue_temp, portMAX_DELAY);
        break;
      case 3: // API返回的文本为空
        queue_temp = 9;
        xQueueSend(OLED_showing_queue, &queue_temp, portMAX_DELAY);
        break;
      case 500: // 长按时间过短
        queue_temp = 6;
        xQueueSend(OLED_showing_queue, &queue_temp, portMAX_DELAY);
        break;
      default: // 返回的是httpcode错误状态码
        queue_temp = 7;
        xQueueSend(OLED_showing_queue, &queue_temp, portMAX_DELAY);
        break;
      }
      free(base64_data);
      base64_data = NULL;
      while (digitalRead(Chat_Key) == 1)
      { // 保持显示识别结果,再次点按回到待机状态
        vTaskDelay(10);
      }
      vTaskDelay(300); // 消抖
      queue_temp = 0;
      xQueueSend(OLED_showing_queue, &queue_temp, portMAX_DELAY); // 告诉OLED显示待机界面
      sis_payload.clear();                                        // 清空一下
    }
    // else if (digitalRead(Command_Key) == 0)
    // {
    //   ESP_LOGI("", "Start listening");
    // }
    else
    {
      vTaskDelay(20);
    }
  }
}


// OLED_task 任务函数
void OLED_task(void *parameter)
{
  uint8_t OLED_showing_state = 4; // OLED待机标志位，置0时显示待机动画
  u8g2.begin();
  u8g2.enableUTF8Print();                                 // 支持显示中文
  xQueueSend(OLED_showing_queue, &OLED_showing_state, 0); // 这里需要先向队列发送一条初始的消息，不然OLED开机没反应
  while (1)
  {
    xQueueReceive(OLED_showing_queue, &OLED_showing_state, 0);
    u8g2.clearBuffer();
    if (OLED_showing_state == 0)
    {
      if ((WiFi.status() == WL_CONNECTED))
      {
        // 显示wifi图标
        u8g2.setFont(u8g2_font_siji_t_6x10);
        u8g2.drawGlyph(115, 9, 57882);
      }
      // 待机动画
      static short i, j;
      static short xTemp = 0;
      static short yTemp = 8 - 1;
      static uint8_t widTemp = 8;
      static uint8_t highTemp = 8;
      u8g2.setDrawColor(1); // 设置有色
      i = xTemp;
      j = 8 - 1;
      while (j >= yTemp)
      {
        if (j > yTemp)
          i = 16 - 1;
        else
          i = xTemp;
        while (i >= 0)
        {
          u8g2.drawBox(i * widTemp, j * highTemp, widTemp, highTemp); // 绘制方块
          i--;
        }
        j--;
      }
      if (yTemp > 1 || xTemp < 16 - 1)
      {
        if (xTemp < 16 - 1)
          xTemp++;
        else
        {
          xTemp = 0;
          yTemp--;
        }
      }
      else
      {
        xTemp = 0;
        yTemp = 8 - 1;
        u8g2.sendBuffer(); // 同步屏幕
        vTaskDelay(100);
        u8g2.clearBuffer(); // clear the u8g2 buffer
        if ((WiFi.status() == WL_CONNECTED))
        {
          // 显示wifi图标
          u8g2.setFont(u8g2_font_siji_t_6x10);
          u8g2.drawGlyph(115, 9, 57882);
        }

        u8g2.sendBuffer(); // 同步屏幕
        vTaskDelay(100);
      }
    }
    else if (OLED_showing_state == 1)
    {
      u8g2.setFont(u8g2_font_wqy12_t_gb2312);
      u8g2.setCursor(40, 10); // 设置坐标
      u8g2.print("录音中~~");
      u8g2.setCursor(30, 30); // 设置坐标
      u8g2.print("松手录音结束……");
      u8g2.setFont(u8g2_font_siji_t_6x10);
      u8g2.drawGlyphX2(50, 60, 57420);
    }
    else if (OLED_showing_state == 2)
    {
      u8g2.setFont(u8g2_font_wqy12_t_gb2312);
      u8g2.setCursor(40, 25); // 设置坐标
      u8g2.print("识别中~~");
    }
    else if (OLED_showing_state == 3)
    {
      static uint32_t k = 0;
      static uint8_t x_add = 0;
      static uint16_t y_add = 0;
      static uint32_t y_err = 0; // 控制所有字体上移
      static bool temp_flag = false;
      y_err = 0;
      u8g2.setFont(u8g2_font_wqy12_t_gb2312);
      do
      {
        u8g2.clearBuffer();
        for (k = 0, x_add = 0, y_add = 0; k < chat_content.length() - 1; k++)
        {
          if (10 - y_err > 0)
          {
            u8g2.setCursor(0, 10 - y_err); // 设置坐标
            u8g2.print("========Rowling=======");
          }
          if (25 - y_err + y_add * 12 > 0)
          {
            u8g2.setCursor(3 + 6 * x_add, 25 - y_err + y_add * 12); // 设置坐标
            if (chat_content[k] > 127)
            {
              // 是中文
              u8g2.print(chat_content[k++]);
              u8g2.print(chat_content[k++]);
              u8g2.print(chat_content[k]);
              x_add += 2;
              temp_flag = true; // 允许刷新屏幕
            }
            else
            {
              // u8g2.setFont(u8g2_font_samim_16_t_all);
              u8g2.print(chat_content[k]);
              temp_flag = true; // 允许刷新屏幕
              x_add++;
            }
            if (x_add >= 20)
            {
              y_add++;
              x_add = 0;
            }
          }
          else if (chat_content[k] > 127)
            k += 2;
        }
        if (temp_flag)
        {
          temp_flag = false;
          u8g2.sendBuffer(); // 同步屏幕
          vTaskDelay(2 / portTICK_PERIOD_MS);
        }
        y_err++;
      } while (25 - y_err + y_add * 12 >= 63);
    }
    else if (OLED_showing_state == 4)
    {
      if ((WiFi.status() == WL_CONNECTED))
      {
        // 显示wifi图标
        u8g2.setFont(u8g2_font_siji_t_6x10);
        u8g2.drawGlyph(115, 9, 57882);
      }
      u8g2.setFont(u8g2_font_wqy12_t_gb2312);
      u8g2.setCursor(25, 30); // 设置坐标
      u8g2.print("组件准备中~~");
    }
    else if (OLED_showing_state == 5)
    {
      u8g2.setFont(u8g2_font_wqy12_t_gb2312);
      u8g2.setCursor(30, 20); // 设置坐标
      u8g2.print("没网了？");
      u8g2.setCursor(5, 32); // 设置坐标
      u8g2.print("请打开wifi:");
      u8g2.print(wifi_ssid);
      u8g2.setCursor(3, 44); // 设置坐标
      u8g2.print("或BLE设置新wifi");
    }
    else if (OLED_showing_state == 6)
    {
      u8g2.setFont(u8g2_font_wqy12_t_gb2312);
      u8g2.setCursor(30, 40); // 设置坐标
      u8g2.print("请重新操作！");
    }
    else if (OLED_showing_state == 7)
    {
      u8g2.setFont(u8g2_font_wqy12_t_gb2312);
      u8g2.setCursor(20, 25); // 设置坐标
      u8g2.print("网络质量不佳qwq");
      u8g2.setCursor(25, 45); // 设置坐标
      u8g2.print("请重新操作QAQ");
    }
    else if (OLED_showing_state == 8)
    {
      u8g2.setFont(u8g2_font_wqy12_t_gb2312);
      u8g2.setCursor(50, 22); // 设置坐标
      u8g2.print("O.o");
      u8g2.setCursor(40, 35); // 设置坐标
      u8g2.print("内存不足！");
      u8g2.setCursor(10, 55); // 设置坐标
      u8g2.print("请联系作者debug");
    }
    else if (OLED_showing_state == 9)
    {
      u8g2.setFont(u8g2_font_wqy12_t_gb2312);
      u8g2.setCursor(25, 40); // 设置坐标
      u8g2.print("你说话了O.o？");
    }
    else if (OLED_showing_state == 10)
    {
      static uint8_t m;
      static uint8_t x_add;
      static uint8_t y_add;
      static String str;
      str = "SSID：";
      str += wifi_ssid;
      u8g2.setFont(u8g2_font_wqy12_t_gb2312);
      u8g2.setCursor(10, 12); // 设置坐标
      u8g2.print("正在尝试链接Wifi~~");
      for (m = 0, x_add = 0, y_add = 0; m < str.length(); m++)
      {
        u8g2.setCursor(0 + 6 * x_add, 26 + y_add * 12); // 设置坐标
        if (str[m] > 127)
        {
          // 是中文
          u8g2.print(str[m++]);
          u8g2.print(str[m++]);
          u8g2.print(str[m]);
          x_add += 2;
        }
        else
        {
          u8g2.print(str[m]);
          x_add++;
        }
        if (x_add >= 20)
        {
          y_add++;
          x_add = 0;
        }
      }
      u8g2.setCursor(0, 60); // 设置坐标
    }
    u8g2.sendBuffer(); // 同步屏幕
  }
}

// LED_task 任务函数
void LED_task(void *parameter)
{
  uint8_t count = 0;
  uint8_t inver_flag = 1;
  while (1)
  {
    LEDB_OFF;
    LEDA_ON;
    vTaskDelay(4 * count + 50);
    LEDB_ON;
    LEDA_OFF;
    vTaskDelay(4 * count + 50);
    if (inver_flag)
    {
      count++;
      if (count == 50)
        inver_flag = 0;
    }
    else
    {
      count--;
      if (count == 0)
        inver_flag = 1;
    }
  }
}

// 定时器0 中断函数，1ms触发一次
void IRAM_ATTR Timer0_Handler()
{
  static uint16_t millisecond = 0;
  millisecond++;
  if (millisecond == 1000)
  {
    // 每秒执行以下内容
    millisecond = 0;
  }
}
