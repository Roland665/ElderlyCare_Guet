#include "myLED.h"


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// LED 控制任务
void LED_task(void *pvParameters){

  while(1){
    LED_RGB_B_OFF;
    LED_RGB_R_ON;
    vTaskDelay(500/portTICK_PERIOD_MS);
    LED_RGB_R_OFF;
    LED_RGB_G_ON;
    vTaskDelay(500/portTICK_PERIOD_MS);
    LED_RGB_G_OFF;
    LED_RGB_B_ON;
    vTaskDelay(500/portTICK_PERIOD_MS);
  }
}

// 初始化所有LED
void LEDS_Init(void){
	LED_RGB_Init();
  xTaskCreate(LED_task, "LED_task", 1024*1, NULL, 13, NULL);
}

// 初始化彩色 LED
void LED_RGB_Init(void){
  gpio_config_t RGB_cfg = {
    .intr_type = GPIO_INTR_DISABLE, // no interrupt
    .mode = GPIO_MODE_OUTPUT,
    .pin_bit_mask = (1UL<<LED_RGB_R_IO | 1UL<<LED_RGB_G_IO | 1UL<<LED_RGB_B_IO),
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .pull_up_en = GPIO_PULLUP_DISABLE
  };
  gpio_config(&RGB_cfg);
  LED_RGB_R_OFF;
  LED_RGB_G_OFF;
  LED_RGB_B_OFF;
}
