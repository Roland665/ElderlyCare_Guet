#include "Zigbee.h"
#include "usart.h"
#include "delay.h"
#include "led/led.h"
#include "timer/timer.h"
#include "DHT11/DHT11.h"

#define wait delay(100);

/**
 * @brief		切换Zigbee模式
 * @param		ModeNum -> 0为进入HEX指令模式，1为进入透传模式
 * @retval		void
 */

void Zigbee_Change_Mode(Zigbee *zigbee, uint8_t modeNum)
{
  uint8_t EnterMode0[] = {0x2B, 0x2B, 0x2B};                                     // Zigbee透传模式切换HEX指令模式 0为HEX指令模式
  uint8_t EnterMode1[] = {0x55, 0x07, 0x00, 0x11, 0x00, 0x03, 0x00, 0x01, 0x13}; // Zigbee HEX指令模式切换透传模式 1为数据透传模式
  uint8_t i;
  if (modeNum == 0)
  {
    while (zigbee->modeFlag != 0)
    {
      wait;
      for (i = 0; i < 3; i++)
      {
        USART_SendData(USART1, EnterMode0[i]); // 向串口1发送数据
        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET)
          ; // 等待发送结束
      }
    }
  }
  else if (modeNum == 1)
  {
    // 进入透传模式
    while (zigbee->modeFlag != 1)
    {
      wait;
      for (i = 0; i < 9; i++)
      {
        USART_SendData(USART1, EnterMode1[i]); // 向串口1发送数据
        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET)
          ; // 等待发送结束
      }
    }
  }
}

/**
 * @brief		设置目标短地址为0x00 0x00(即协调器)和目标端口为01
 * @param		void
 * @retval		void
 */

void Zigbee_Set_Send_Target(Zigbee *zigbee)
{
  uint8_t i;
  uint8_t SetDirection[] = {0x55, 0x08, 0x00, 0x11, 0x00, 0x01, 0x00, 0x00, 0x00, 0x10, 0x55, 0x07, 0x00, 0x11, 0x00, 0x02, 0x00, 0x01, 0x12};
  zigbee->readySetTargetFlag = 0;
  zigbee->setSendTargetFlag = 0;
  while (zigbee->setSendTargetFlag != 1)
  {
    wait;
    for (i = 0; i < 10; i++)
    {
      USART_SendData(USART1, SetDirection[i]); // 向串口1发送数据
      while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET)
        ; // 等待发送结束
    }
  }
  while (zigbee->setSendTargetFlag != 0)
  {
    wait;
    for (i = 10; i < 19; i++)
    {
      USART_SendData(USART1, SetDirection[i]); // 向串口1发送数据
      while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET)
        ; // 等待发送结束
    }
  }
  zigbee->readySetTargetFlag = 1;
}

/**
 * @brief		打开Zigbee模块的网络（终端：加入一个指定zigbee->PANID的现有网络）
 * @param		void
 * @retval	    void
 */

void Zigbee_Open_Net(Zigbee *zigbee)
{
  uint8_t i;
  uint8_t NetStart[] = {
      0x55,
      0x04,
      0x00,
      0x02,
      0x00,
      0x02,
  };
  zigbee->openNetFlag = 0;
  while (zigbee->openNetFlag != 1)
  {
    wait;
    for (i = 0; i < 6; i++)
    {
      USART_SendData(USART1, NetStart[i]); // 向串口1发送数据
      while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET)
        ; // 等待发送结束
    }
  }
}

/**
 * @brief		打开Zigbee模块的网络（终端：加入一个指定zigbee->PANID的现有网络）
 * @param		void
 * @retval	    void
 */

void Zigbee_Close_Net(Zigbee *zigbee)
{
  uint8_t i;
  uint8_t NetClose[] = {0x55, 0x03, 0x00, 0x03, 0x03};
  zigbee->openNetFlag = 0;
  wait;
  for (i = 0; i < 5; i++)
  {
    USART_SendData(USART1, NetClose[i]); // 向串口1发送数据
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET)
      ; // 等待发送结束
  }
  wait;
  for (i = 0; i < 5; i++)
  {
    USART_SendData(USART1, NetClose[i]); // 向串口1发送数据
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET)
      ; // 等待发送结束
  }
}

/**
 * @brief		查询模组当前状态（读取参数）
 * @param		void
 * @retval	    1->成功获取状态
 */
void Zigbee_Get_State(Zigbee *zigbee)
{
  uint8_t i;
  uint8_t GetState[] = {0x55, 0x03, 0x00, 0x00, 0x00}; // 查询Zigbee模组当前状态
  zigbee->getStateFlag = 0;
  while (zigbee->getStateFlag != 1)
  {
    wait;
    for (i = 0; i < 5; i++)
    {
      USART_SendData(USART1, GetState[i]); // 向串口1发送数据
      while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET)
        ; // 等待发送结束
    }
  }
}

/**
 * @brief		向中控发送需要应答的心跳包
 */

void Zigbee_Update_TerminalOnlineFlag(Zigbee *zigbee)
{
  uint8_t wait_count = 0;
  uint8_t HBPack[1 + 2 + 8 + 1];
  uint8_t i = 0;
  HBPack[i++] = 0x02; // 这里一定要修改为对应终端的设备类型码
  HBPack[i++] = zigbee->SAddr[0];
  HBPack[i++] = zigbee->SAddr[1];
  HBPack[i++] = 1; // 需应答
  for (uint8_t j = 0; j < 2; j++)
    HBPack[i++] = PWMval[j];
  for (uint8_t j = 0; j < 4; j++)
    HBPack[i++] = dht11_data[j];
  zigbee->ackFlag = 0;
  while (zigbee->ackFlag != 1)
  {
    send_customFrame(zigbee, 0xFE, 1 + 2 + 1 + 6, HBPack);
    delay(700); // 考虑数据接收延迟,避免频繁发送导致中控数据拥堵
    set_LED1;
    if (wait_count >= 3)
    { // 3s发三次没收到应答，直接退出，表示为没有入网
      zigbee->terminalOnlineFlag = 0;
      return;
    }
    delay(300); // 考虑数据接收延迟,避免频繁发送导致中控数据拥堵
    reset_LED1;
    wait_count++;
  }
  zigbee->terminalOnlineFlag = 1;
}

/**
 * @brief		模组恢复出厂设置
 * @param		void
 * @retval		void
 */

void Zigbee_Restore_Factory_Setting(Zigbee *zigbee)
{
  uint8_t i;
  uint8_t checkCode = 0;
  uint8_t RestoreFactorySetting[] = {0x55, 0x07, 0x00, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00}; // 恢复出厂指令
  RestoreFactorySetting[5] = zigbee->PANID[0];
  RestoreFactorySetting[6] = zigbee->PANID[1];
  RestoreFactorySetting[7] = zigbee->channel;
  // 计算上面命令的BBC校验码
  for (i = 0; i < RestoreFactorySetting[1] - 1; i++)
  {
    checkCode = checkCode ^ RestoreFactorySetting[2 + i];
  }
  RestoreFactorySetting[8] = checkCode; // 校验码赋值
  wait;
  for (i = 0; i < 9; i++)
  {
    USART_SendData(USART1, RestoreFactorySetting[i]); // 向串口1发送数据
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET)
      ; // 等待发送结束
  }
  delay(1000); // 等待模块正常工作
}

/**
 * @brief		模组复位(恢复出厂后打开网络前必须重启)
 * @param		void
 * @retval		void
 */

void Zigbee_Restart(Zigbee *zigbee)
{
  uint8_t i;
  uint8_t Restart[] = {0x55, 0x07, 0x00, 0x04, 0x00, 0xFF, 0xFF, 0x00, 0x04}; // 恢复出厂指令
  wait;
  for (i = 0; i < 9; i++)
  {
    USART_SendData(USART1, Restart[i]); // 向串口1发送数据
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET)
      ; // 等待发送结束
  }
  wait;
  for (i = 0; i < 9; i++)
  {
    USART_SendData(USART1, Restart[i]); // 向串口1发送数据
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET)
      ; // 等待发送结束
  }
  // 发两次保证执行成功
}

/**
 * @brief		设置模组类型为终端
 * @param		void
 * @retval		void
 */

void Zigbee_Set_Type_To_Active_Terminal(Zigbee *zigbee)
{
  uint8_t i;
  uint8_t Set_Type_To_Terminal[] = {0x55, 0x04, 0x00, 0x05, 0x02, 0x07}; // 设置模组类型为终端指令
  zigbee->setTypeFlag = 0;
  while (zigbee->setTypeFlag != 1)
  {
    wait;
    for (i = 0; i < 6; i++)
    {
      USART_SendData(USART1, Set_Type_To_Terminal[i]); // 向串口1发送数据
      while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET)
        ; // 等待发送结束
    }
  }
}

/**
 * @brief		分析Zigbee的反馈命令
 * @param		void
 * @retval		void
 */
void Zigbee_Analyse_Command_Data(Zigbee *zigbee)
{
  if (model == E18)
  {
    if (usart1_rx_buffer[2] == 0x00 && usart1_rx_buffer[3] == 0x00)
    { // 55 2A 00 00 00
      if (usart1_rx_buffer[4] == 0x00)
      { // 已配网获取一波设备信息
        zigbee->LAddr[0] = usart1_rx_buffer[6];
        zigbee->LAddr[1] = usart1_rx_buffer[7];
        zigbee->LAddr[2] = usart1_rx_buffer[8];
        zigbee->LAddr[3] = usart1_rx_buffer[9];
        zigbee->LAddr[4] = usart1_rx_buffer[10];
        zigbee->LAddr[5] = usart1_rx_buffer[11];
        zigbee->LAddr[6] = usart1_rx_buffer[12];
        zigbee->LAddr[7] = usart1_rx_buffer[13];
        zigbee->channel = usart1_rx_buffer[14];
        zigbee->PANID[0] = usart1_rx_buffer[15];
        zigbee->PANID[1] = usart1_rx_buffer[16];
        zigbee->SAddr[0] = usart1_rx_buffer[17];
        zigbee->SAddr[1] = usart1_rx_buffer[18];
        zigbee->getStateFlag = 1;
        zigbee->zigbeeOnlineFlag = 1; // 已入网
      }
      else if (usart1_rx_buffer[4] == 0xFF)
      { // 未组网
        zigbee->LAddr[0] = usart1_rx_buffer[6];
        zigbee->LAddr[1] = usart1_rx_buffer[7];
        zigbee->LAddr[2] = usart1_rx_buffer[8];
        zigbee->LAddr[3] = usart1_rx_buffer[9];
        zigbee->LAddr[4] = usart1_rx_buffer[10];
        zigbee->LAddr[5] = usart1_rx_buffer[11];
        zigbee->LAddr[6] = usart1_rx_buffer[12];
        zigbee->LAddr[7] = usart1_rx_buffer[13];
        zigbee->getStateFlag = 1;
        zigbee->zigbeeOnlineFlag = 0; // 未入网
      }
    }
    else if (usart1_rx_buffer[1] == 0x04 && usart1_rx_buffer[2] == 0x00 && usart1_rx_buffer[3] == 0x02 && usart1_rx_buffer[4] == 0x00 && usart1_rx_buffer[5] == 0x02)
    {                          // 55 04 00 02 00 02
      zigbee->openNetFlag = 1; // 判断网络已打开
    }
    else if (usart1_rx_buffer[1] == 0x04 && usart1_rx_buffer[2] == 0x00 && usart1_rx_buffer[3] == 0x05 && usart1_rx_buffer[4] == 0x00 && usart1_rx_buffer[5] == 0x05)
    {                          // 55 04 00 05 00 05
      zigbee->setTypeFlag = 1; // 判断成功设置终端类型
    }
    else if (zigbee->readySetTargetFlag == 1 && usart1_rx_buffer[1] == 0x04 && usart1_rx_buffer[2] == 0x00 && usart1_rx_buffer[3] == 0x11 && usart1_rx_buffer[4] == 0x00 && usart1_rx_buffer[5] == 0x11)
    { // 55 04 00 11 00 11
      zigbee->modeFlag = 1;
    }
    else if (zigbee->readySetTargetFlag == 0 && usart1_rx_buffer[1] == 0x04 && usart1_rx_buffer[2] == 0x00 && usart1_rx_buffer[3] == 0x11 && usart1_rx_buffer[4] == 0x00 && usart1_rx_buffer[5] == 0x11)
    { // 55 04 00 11 00 11
      if (zigbee->setSendTargetFlag == 0)
        zigbee->setSendTargetFlag = 1;
      else
        zigbee->setSendTargetFlag = 0;
    }
    else if (usart1_rx_buffer[1] == 0x03 && usart1_rx_buffer[2] == 0xFF && usart1_rx_buffer[3] == 0xFE && usart1_rx_buffer[4] == 0x01)
    { // 55 03 FF FE 01
      if (zigbee->modeFlag != 0)
        zigbee->modeFlag = 0;
    }
  }
  else if (model == E180)
  {
  }
}
