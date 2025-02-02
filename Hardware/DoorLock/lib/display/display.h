#pragma once

#include <lvgl.h>

#define LCD_LED_PIN 3
#define LCD_LED_PWM_CHANNEL 0

#define BTN_WIDTH  100
#define BTN_HEIGHT 50
#define MARGIN     6

const uint16_t screenWidth = 480; // 像素宽
const uint16_t screenHeight = 320; // 像素高

// 触摸键盘文本内容
static const char *btnm_texts[] = {"1", "2", "3", "Clear", "\n",
                                "4", "5", "6", LV_SYMBOL_BACKSPACE, "\n",
                                "7", "8", "9", "Close", "\n",
                                LV_SYMBOL_EYE_CLOSE, "0", LV_SYMBOL_EYE_OPEN, LV_SYMBOL_NEW_LINE};


class Display
{
private:
  lv_obj_t * m_ta;// 基于 show_keyboard 方法创建的文本框对象
public:
	void init(void);
	void run(void);
	void setBackLight(float);
  void show_keyboard(void);
  void show_progress_InKeyboard(uint8_t progress);
};

extern Display *screen;

