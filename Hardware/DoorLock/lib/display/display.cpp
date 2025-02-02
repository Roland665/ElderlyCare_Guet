#include <stdbool.h>
#include <TFT_eSPI.h>
#include <Arduino.h>

#include "display.h"
#include "mytasks.h"
TFT_eSPI tft = TFT_eSPI();

Display *screen = new Display();
#if LV_USE_LOG != 0
/* 使用串口打印调试 */
static void lv_print(const char *buf)
{
  Serial.printf(buf);
  Serial.flush();
}
#endif

/* 刷新显示器 */
static void disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors(&color_p->full, w * h, true);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

/* 读取触屏信息 */
static void touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  uint16_t touchX, touchY;

  bool touched = tft.getTouch(&touchX, &touchY, 600);

  if (!touched)
    data->state = LV_INDEV_STATE_REL;
  else
  {
    data->state = LV_INDEV_STATE_PR;

    /*Set the coordinates*/
    data->point.x = touchX;
    data->point.y = touchY;
    ESP_LOGD("", "touchX:%d\ttouchY: %d\n", touchX, touchY);
  }
}

/* 触摸键盘回调函数 */
static void keyboard_event_handler(lv_event_t *e)
{
  lv_obj_t * btn = lv_event_get_target(e); // 获取按钮矩阵中被按下的按钮对象
  lv_obj_t * ta = (lv_obj_t *)lv_event_get_user_data(e); // 获取捆绑的文本框对象
  const char * text = lv_btnmatrix_get_btn_text(btn, lv_btnmatrix_get_selected_btn(btn)); // 获取被按下的按钮对象的文本

  for(uint16_t i = 1; i <= 19; i++)
    if(strcmp(text, btnm_texts[i-1]) == 0){
      i = i - i/5;
      xQueueSend(input_queue, &i, portMAX_DELAY);
      break;
    }

  if(strcmp(text, LV_SYMBOL_BACKSPACE) == 0)
    lv_textarea_del_char(ta); // 退格
  else if(strcmp(text, LV_SYMBOL_NEW_LINE) == 0 || strcmp(text, "Clear") == 0)
    lv_textarea_set_text(ta, ""); // 清空文本框
  else if(text[0] <= '9' && text[0] >= '0')
    lv_textarea_add_text(ta, text); // 添加文本
}

void Display::init(void)
{
  // /* 设置屏幕亮度 */
  // ledcSetup(LCD_LED_PWM_CHANNEL, 5000, 8);
  // ledcAttachPin(LCD_LED_PIN, LCD_LED_PWM_CHANNEL);

  /* 初始化 lvgl */
  lv_init();

  // 提供 lvgl 串口调试
#if LV_USE_LOG != 0
  lv_log_register_print_cb(lv_print); /* register print function for debugging */
#endif

  /* 初始化显示设备 */
  tft.begin();
  tft.setRotation(1); /* 旋转 */

  /* 校准触摸屏 */
  uint16_t calData[] = {248, 3678, 174, 3707, 7};
  // uint16_t calData[] = {275, 3620, 264, 3532, 7};
  tft.setTouch(calData);

  /* 创建lvgl缓冲区 */
  static lv_disp_draw_buf_t draw_buf;                       // 缓冲区数据结构
#define LV_BUF_SIZE screenHeight *screenWidth / 10          // lvgl缓冲区大小
  static lv_color_t buf[LV_BUF_SIZE];                       // 缓冲区
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, LV_BUF_SIZE); /*Initialize the display buffer*/

  /* 初始化显示驱动 */
  static lv_disp_drv_t disp_drv; // 驱动的数据结构
  lv_disp_drv_init(&disp_drv);   // 初始化数据结构
  // 设置显示驱动的显示分辨率
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = disp_flush;  // 提供刷新显示器的函数入口
  disp_drv.draw_buf = &draw_buf;   // 提供显示器缓冲区
  lv_disp_drv_register(&disp_drv); // 注册驱动

  /* 初始化输入设备驱动 */
  static lv_indev_drv_t indev_drv;        // 驱动的数据结构
  lv_indev_drv_init(&indev_drv);          // 初始化数据结构
  indev_drv.type = LV_INDEV_TYPE_POINTER; // 选择输入设备类型
  indev_drv.read_cb = touchpad_read;      // 提供读取输入设备的函数入口
  lv_indev_drv_register(&indev_drv);      // 注册驱动

  ESP_LOGI("", "Lvgl init");
}

// 确保 lvgl 可以正常工作
void Display::run(void)
{
  lv_task_handler();
}

// 设置亮度百分比
void Display::setBackLight(float duty)
{
  duty = constrain(duty, 0, 1);
  duty = 1 - duty;
  ledcWrite(LCD_LED_PWM_CHANNEL, (int)(duty * 255));
}

// 从x,y坐标开始显示一个键盘
void Display::show_keyboard(void)
{
  /* 实例化文本框对象 */
  m_ta = lv_textarea_create(lv_scr_act());
  lv_textarea_set_one_line(m_ta, true); // 设置为单行文本
  lv_obj_set_size(m_ta, 380, 40);
  lv_obj_align(m_ta, LV_ALIGN_TOP_MID, 0, 10); // 向顶中对齐，微调y轴偏移
  lv_obj_add_state(m_ta, LV_STATE_FOCUSED); // 设置文本框的光标可见

  lv_obj_t *btnm;
  btnm = lv_btnmatrix_create(lv_scr_act()); // 创建按钮矩阵
  lv_obj_set_size(btnm, 380, 240);
  lv_obj_align(btnm, LV_ALIGN_BOTTOM_MID, 0, -5); // 对齐
  lv_obj_add_event_cb(btnm, keyboard_event_handler, LV_EVENT_VALUE_CHANGED, m_ta); // 事件回调函数，具备长按识别
  lv_obj_clear_flag(btnm, LV_OBJ_FLAG_CLICK_FOCUSABLE); /*To keep the text area focused on button clicks*/
  lv_btnmatrix_set_map(btnm, btnm_texts);
}

// 基于 show_keyboard 方法创建的文本框显示进度条
// progress: 进度百分比，50%即progress=50
#define PROGRESS_MAX 44.0
void Display::show_progress_InKeyboard(uint8_t progress)
{
  if(progress < 100){
    char text[45+1];
    uint8_t i = 0;
    for(; i < PROGRESS_MAX/100*progress; i++)
      text[i] = '=';
    text[i++] = '>';
    text[i] = 0;
    lv_textarea_set_text(m_ta, text);
  }
  else
    lv_textarea_set_text(m_ta, "Over~");
}
