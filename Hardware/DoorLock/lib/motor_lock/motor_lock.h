#pragma once

#define MOTOR_PWM_PIN     1
#define MOTOR_PWM_CHANNAL 1

extern bool lock_state;


void lock_init(float duty);
void locking(void);
void unlocking(void);
