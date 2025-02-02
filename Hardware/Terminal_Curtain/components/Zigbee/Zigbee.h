#ifndef __ZIGBEE_H
#define __ZIGBEE_H
#include "stm32f10x.h"

#define E18  1
#define E180 2

#define model E18 //这里选择一下使用的模块时E180还是E18

typedef struct _Zigbee{
  uint8_t LAddr[8];//Zigbee设备长地址
  uint8_t SAddr[2];//Zigbee设备短地址 协调器初始化为0xFF 0xFF，终端初始化为0
  uint8_t modeFlag;//Zigbee模块模式切换标志位，进入配置模式置0，透传模式置1
  uint8_t PANID[2];//Zigbee局网PANID,初始化为0x00 0x00
  uint8_t openNetFlag;//打开网络标志位，打开成功置1
  uint8_t getStateFlag;//读取模块状态标志位，读取成功置1
  uint8_t readySetTargetFlag;//成功设置透传目标标志位，设置成功置1(与协调器的同名变量作用有一定出入)
  uint8_t setSendTargetFlag;//设置透传目标标志位，分两步，先设置目标短地址，再设置目标端口，短地址设置成功后置1，端口设置成功后置0
  uint8_t setTypeFlag;//成功设置设备类型为终端标志位，设置成功置1
  uint8_t channel;//Zigbee模块信道，配网后才能读取
  uint8_t zigbeeOnlineFlag; // Zigbee在网标志，在网时置一
  uint8_t terminalOnlineFlag;//在网Flag,置0表示没有连上生态,置1表示已连上
  uint8_t ackFlag; // 应答标志位，被中控应答时置一
}Zigbee;


void Zigbee_Change_Mode(Zigbee *zigbee, uint8_t modeNum);
void Zigbee_Set_Send_Target(Zigbee *zigbee);
void Zigbee_Open_Net(Zigbee *zigbee);
void Zigbee_Get_State(Zigbee *zigbee);
void Zigbee_Restore_Factory_Setting(Zigbee *zigbee);
void Zigbee_Restart(Zigbee *zigbee);
void Zigbee_Set_Type_To_Active_Terminal(Zigbee *zigbee);
void Zigbee_Analyse_Command_Data(Zigbee *zigbee);
void Zigbee_Close_Net(Zigbee *zigbee);
void Zigbee_Update_TerminalOnlineFlag(Zigbee *zigbee);
#endif
