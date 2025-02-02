#include "motor_lock.h"
#include <Arduino.h>

bool lock_state = false; // true-锁已开，false-锁已关

// 初始化门锁舵机，并附带一定占空比
void lock_init(float duty){
  ledcSetup(MOTOR_PWM_CHANNAL, 50, 12);
  ledcAttachPin(MOTOR_PWM_PIN, MOTOR_PWM_CHANNAL);
  ledcWrite(MOTOR_PWM_CHANNAL, uint32_t(0.025 * 0xFFF));
}

void locking(void){
  lock_state = false;
  ledcWrite(MOTOR_PWM_CHANNAL, uint32_t(0.025 * 0xFFF));
  ESP_LOGI("", "Locking qwq");
}
void unlocking(void){
  lock_state = true;
  ledcWrite(MOTOR_PWM_CHANNAL, uint32_t(0.075 * 0xFFF));
  ESP_LOGI("", "Unlocking！！！");
}
