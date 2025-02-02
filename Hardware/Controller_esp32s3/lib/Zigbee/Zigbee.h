#pragma once
#include <Arduino.h>

class Zigbee{
private:
  HardwareSerial *m_serial;
  uint8_t m_longAddr[8];
  uint8_t m_shortAddr[2];
  uint8_t m_modeFlag = -1; // 当前Zigbee模块的状态标志 0-配置模式，1-传输模式
  uint8_t m_openNetFlag = 0; // 置一表示成功打开网络
  uint8_t m_getStateFlag = 0; // 置一表示成功查询Zigbee模块状态
  uint8_t m_setTargetReadyFlag = 1; // 置一表示透传目标已确定
  uint8_t m_setTargetStep = 0; // 0-透传目标第一步(设置短地址)暂未成功，1-透传目标第二步(设置端口)暂未成功
public:
  Zigbee(HardwareSerial *serial);
  void analyze_feedback(uint8_t *buffer);
  void openNet(void);
  void change_mode(uint8_t mode);
  void get_state(void);
  void set_send_target(uint8_t *DSAddr,uint8_t DPort);
  void start(void);
  void send(uint8_t *buffer, uint32_t length);
  uint8_t get_modeFlag(void);
};
