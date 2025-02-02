#ifndef __ZIGBEE_H
#define __ZIGBEE_H
#include "stm32f10x.h"

#define E18  1
#define E180 2

#define model E18 //����ѡ��һ��ʹ�õ�ģ��ʱE180����E18

typedef struct _Zigbee{
  uint8_t LAddr[8];//Zigbee�豸����ַ
  uint8_t SAddr[2];//Zigbee�豸�̵�ַ Э������ʼ��Ϊ0xFF 0xFF���ն˳�ʼ��Ϊ0
  uint8_t modeFlag;//Zigbeeģ��ģʽ�л���־λ����������ģʽ��0��͸��ģʽ��1
  uint8_t PANID[2];//Zigbee����PANID,��ʼ��Ϊ0x00 0x00
  uint8_t openNetFlag;//�������־λ���򿪳ɹ���1
  uint8_t getStateFlag;//��ȡģ��״̬��־λ����ȡ�ɹ���1
  uint8_t readySetTargetFlag;//�ɹ�����͸��Ŀ���־λ�����óɹ���1(��Э������ͬ������������һ������)
  uint8_t setSendTargetFlag;//����͸��Ŀ���־λ����������������Ŀ��̵�ַ��������Ŀ��˿ڣ��̵�ַ���óɹ�����1���˿����óɹ�����0
  uint8_t setTypeFlag;//�ɹ������豸����Ϊ�ն˱�־λ�����óɹ���1
  uint8_t channel;//Zigbeeģ���ŵ�����������ܶ�ȡ
  uint8_t zigbeeOnlineFlag; // Zigbee������־������ʱ��һ
  uint8_t terminalOnlineFlag;//����Flag,��0��ʾû��������̬,��1��ʾ������
  uint8_t ackFlag; // Ӧ���־λ�����п�Ӧ��ʱ��һ
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
