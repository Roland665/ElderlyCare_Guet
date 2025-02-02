#pragma once

#include "driver/gpio.h"

#define LED_RGB_R_IO  GPIO_NUM_2
#define LED_RGB_G_IO  GPIO_NUM_13
#define LED_RGB_B_IO  GPIO_NUM_15


#define LED_RGB_R_OFF gpio_set_level(LED_RGB_R_IO, 1)
#define LED_RGB_G_OFF gpio_set_level(LED_RGB_G_IO, 1)
#define LED_RGB_B_OFF gpio_set_level(LED_RGB_B_IO, 1)

#define LED_RGB_R_ON gpio_set_level(LED_RGB_R_IO, 0)
#define LED_RGB_G_ON gpio_set_level(LED_RGB_G_IO, 0)
#define LED_RGB_B_ON gpio_set_level(LED_RGB_B_IO, 0)

void LEDS_Init(void);
void LED_RGB_Init(void);
