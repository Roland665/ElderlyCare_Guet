#include "Zigbee.h"
#include "usart.h"
#include "delay.h"
#include "led/led.h"
#include "timer/timer.h"
#include "DHT11/DHT11.h"

#define wait delay(100);

/**
 * @brief		�л�Zigbeeģʽ
 * @param		ModeNum -> 0Ϊ����HEXָ��ģʽ��1Ϊ����͸��ģʽ
 * @retval		void
 */

void Zigbee_Change_Mode(Zigbee *zigbee, uint8_t modeNum)
{
  uint8_t EnterMode0[] = {0x2B, 0x2B, 0x2B};                                     // Zigbee͸��ģʽ�л�HEXָ��ģʽ 0ΪHEXָ��ģʽ
  uint8_t EnterMode1[] = {0x55, 0x07, 0x00, 0x11, 0x00, 0x03, 0x00, 0x01, 0x13}; // Zigbee HEXָ��ģʽ�л�͸��ģʽ 1Ϊ����͸��ģʽ
  uint8_t i;
  if (modeNum == 0)
  {
    while (zigbee->modeFlag != 0)
    {
      wait;
      for (i = 0; i < 3; i++)
      {
        USART_SendData(USART1, EnterMode0[i]); // �򴮿�1��������
        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET)
          ; // �ȴ����ͽ���
      }
    }
  }
  else if (modeNum == 1)
  {
    // ����͸��ģʽ
    while (zigbee->modeFlag != 1)
    {
      wait;
      for (i = 0; i < 9; i++)
      {
        USART_SendData(USART1, EnterMode1[i]); // �򴮿�1��������
        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET)
          ; // �ȴ����ͽ���
      }
    }
  }
}

/**
 * @brief		����Ŀ��̵�ַΪ0x00 0x00(��Э����)��Ŀ��˿�Ϊ01
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
      USART_SendData(USART1, SetDirection[i]); // �򴮿�1��������
      while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET)
        ; // �ȴ����ͽ���
    }
  }
  while (zigbee->setSendTargetFlag != 0)
  {
    wait;
    for (i = 10; i < 19; i++)
    {
      USART_SendData(USART1, SetDirection[i]); // �򴮿�1��������
      while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET)
        ; // �ȴ����ͽ���
    }
  }
  zigbee->readySetTargetFlag = 1;
}

/**
 * @brief		��Zigbeeģ������磨�նˣ�����һ��ָ��zigbee->PANID���������磩
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
      USART_SendData(USART1, NetStart[i]); // �򴮿�1��������
      while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET)
        ; // �ȴ����ͽ���
    }
  }
}

/**
 * @brief		��Zigbeeģ������磨�նˣ�����һ��ָ��zigbee->PANID���������磩
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
    USART_SendData(USART1, NetClose[i]); // �򴮿�1��������
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET)
      ; // �ȴ����ͽ���
  }
  wait;
  for (i = 0; i < 5; i++)
  {
    USART_SendData(USART1, NetClose[i]); // �򴮿�1��������
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET)
      ; // �ȴ����ͽ���
  }
}

/**
 * @brief		��ѯģ�鵱ǰ״̬����ȡ������
 * @param		void
 * @retval	    1->�ɹ���ȡ״̬
 */
void Zigbee_Get_State(Zigbee *zigbee)
{
  uint8_t i;
  uint8_t GetState[] = {0x55, 0x03, 0x00, 0x00, 0x00}; // ��ѯZigbeeģ�鵱ǰ״̬
  zigbee->getStateFlag = 0;
  while (zigbee->getStateFlag != 1)
  {
    wait;
    for (i = 0; i < 5; i++)
    {
      USART_SendData(USART1, GetState[i]); // �򴮿�1��������
      while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET)
        ; // �ȴ����ͽ���
    }
  }
}

/**
 * @brief		���пط�����ҪӦ���������
 */

void Zigbee_Update_TerminalOnlineFlag(Zigbee *zigbee)
{
  uint8_t wait_count = 0;
  uint8_t HBPack[1 + 2 + 8 + 1];
  uint8_t i = 0;
  HBPack[i++] = 0x02; // ����һ��Ҫ�޸�Ϊ��Ӧ�ն˵��豸������
  HBPack[i++] = zigbee->SAddr[0];
  HBPack[i++] = zigbee->SAddr[1];
  HBPack[i++] = 1; // ��Ӧ��
  for (uint8_t j = 0; j < 2; j++)
    HBPack[i++] = PWMval[j];
  for (uint8_t j = 0; j < 4; j++)
    HBPack[i++] = dht11_data[j];
  zigbee->ackFlag = 0;
  while (zigbee->ackFlag != 1)
  {
    send_customFrame(zigbee, 0xFE, 1 + 2 + 1 + 6, HBPack);
    delay(700); // �������ݽ����ӳ�,����Ƶ�����͵����п�����ӵ��
    set_LED1;
    if (wait_count >= 3)
    { // 3s������û�յ�Ӧ��ֱ���˳�����ʾΪû������
      zigbee->terminalOnlineFlag = 0;
      return;
    }
    delay(300); // �������ݽ����ӳ�,����Ƶ�����͵����п�����ӵ��
    reset_LED1;
    wait_count++;
  }
  zigbee->terminalOnlineFlag = 1;
}

/**
 * @brief		ģ��ָ���������
 * @param		void
 * @retval		void
 */

void Zigbee_Restore_Factory_Setting(Zigbee *zigbee)
{
  uint8_t i;
  uint8_t checkCode = 0;
  uint8_t RestoreFactorySetting[] = {0x55, 0x07, 0x00, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00}; // �ָ�����ָ��
  RestoreFactorySetting[5] = zigbee->PANID[0];
  RestoreFactorySetting[6] = zigbee->PANID[1];
  RestoreFactorySetting[7] = zigbee->channel;
  // �������������BBCУ����
  for (i = 0; i < RestoreFactorySetting[1] - 1; i++)
  {
    checkCode = checkCode ^ RestoreFactorySetting[2 + i];
  }
  RestoreFactorySetting[8] = checkCode; // У���븳ֵ
  wait;
  for (i = 0; i < 9; i++)
  {
    USART_SendData(USART1, RestoreFactorySetting[i]); // �򴮿�1��������
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET)
      ; // �ȴ����ͽ���
  }
  delay(1000); // �ȴ�ģ����������
}

/**
 * @brief		ģ�鸴λ(�ָ������������ǰ��������)
 * @param		void
 * @retval		void
 */

void Zigbee_Restart(Zigbee *zigbee)
{
  uint8_t i;
  uint8_t Restart[] = {0x55, 0x07, 0x00, 0x04, 0x00, 0xFF, 0xFF, 0x00, 0x04}; // �ָ�����ָ��
  wait;
  for (i = 0; i < 9; i++)
  {
    USART_SendData(USART1, Restart[i]); // �򴮿�1��������
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET)
      ; // �ȴ����ͽ���
  }
  wait;
  for (i = 0; i < 9; i++)
  {
    USART_SendData(USART1, Restart[i]); // �򴮿�1��������
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET)
      ; // �ȴ����ͽ���
  }
  // �����α�ִ֤�гɹ�
}

/**
 * @brief		����ģ������Ϊ�ն�
 * @param		void
 * @retval		void
 */

void Zigbee_Set_Type_To_Active_Terminal(Zigbee *zigbee)
{
  uint8_t i;
  uint8_t Set_Type_To_Terminal[] = {0x55, 0x04, 0x00, 0x05, 0x02, 0x07}; // ����ģ������Ϊ�ն�ָ��
  zigbee->setTypeFlag = 0;
  while (zigbee->setTypeFlag != 1)
  {
    wait;
    for (i = 0; i < 6; i++)
    {
      USART_SendData(USART1, Set_Type_To_Terminal[i]); // �򴮿�1��������
      while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET)
        ; // �ȴ����ͽ���
    }
  }
}

/**
 * @brief		����Zigbee�ķ�������
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
      { // ��������ȡһ���豸��Ϣ
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
        zigbee->zigbeeOnlineFlag = 1; // ������
      }
      else if (usart1_rx_buffer[4] == 0xFF)
      { // δ����
        zigbee->LAddr[0] = usart1_rx_buffer[6];
        zigbee->LAddr[1] = usart1_rx_buffer[7];
        zigbee->LAddr[2] = usart1_rx_buffer[8];
        zigbee->LAddr[3] = usart1_rx_buffer[9];
        zigbee->LAddr[4] = usart1_rx_buffer[10];
        zigbee->LAddr[5] = usart1_rx_buffer[11];
        zigbee->LAddr[6] = usart1_rx_buffer[12];
        zigbee->LAddr[7] = usart1_rx_buffer[13];
        zigbee->getStateFlag = 1;
        zigbee->zigbeeOnlineFlag = 0; // δ����
      }
    }
    else if (usart1_rx_buffer[1] == 0x04 && usart1_rx_buffer[2] == 0x00 && usart1_rx_buffer[3] == 0x02 && usart1_rx_buffer[4] == 0x00 && usart1_rx_buffer[5] == 0x02)
    {                          // 55 04 00 02 00 02
      zigbee->openNetFlag = 1; // �ж������Ѵ�
    }
    else if (usart1_rx_buffer[1] == 0x04 && usart1_rx_buffer[2] == 0x00 && usart1_rx_buffer[3] == 0x05 && usart1_rx_buffer[4] == 0x00 && usart1_rx_buffer[5] == 0x05)
    {                          // 55 04 00 05 00 05
      zigbee->setTypeFlag = 1; // �жϳɹ������ն�����
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
