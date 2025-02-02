#include "stm32f10x.h"
#include "DHT11.h"
#include "usart.h"

void DHT11_IO_IN(void) // ��ʪ��ģ�����뺯��
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
  GPIO_InitStructure.GPIO_Pin = IO_DHT11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIO_DHT11, &GPIO_InitStructure);
}

void DHT11_IO_OUT(void) // ��ʪ��ģ���������
{
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_InitStructure.GPIO_Pin = IO_DHT11;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIO_DHT11, &GPIO_InitStructure);
}

// ��λDHT11
void DHT11_Rst(void)
{
  DHT11_IO_OUT(); // SET OUTPUT
  DHT11_DQ_Low;   // DQ=0
  delay_ms(20);   // ��������18ms
  DHT11_DQ_High;  // DQ=1
  delay_us(30);   // ��������20~40us
}

// �ȴ�DHT11�Ļ�Ӧ
// ����1:δ��⵽DHT11�Ĵ���
// ����0:����
uint8_t DHT11_Check(void)
{
  uint8_t retry = 0;                                                        // ������ʱ����
  DHT11_IO_IN();                                                            // SET INPUT
  while ((GPIO_ReadInputDataBit(GPIO_DHT11, IO_DHT11) == 1) && retry < 100) // DHT11������40~80us
  {
    retry++;
    delay_us(1);
  };
  if (retry >= 100)
    return 1;
  else
    retry = 0;
  while ((GPIO_ReadInputDataBit(GPIO_DHT11, IO_DHT11) == 0) && retry < 100) // DHT11���ͺ���ٴ�����40~80us
  {
    retry++;
    delay_us(1);
  };
  if (retry >= 100)
    return 1;
  return 0;
}
// ��DHT11��ȡһ��λ
// ����ֵ��1/0

uint8_t DHT11_Read_Bit(void)
{
  uint8_t retry = 0;
  while ((GPIO_ReadInputDataBit(GPIO_DHT11, IO_DHT11) == 1) && retry < 100) // �ȴ���Ϊ�͵�ƽ
  {
    retry++;
    delay_us(1);
  }
  retry = 0;
  while ((GPIO_ReadInputDataBit(GPIO_DHT11, IO_DHT11) == 0) && retry < 100) // �ȴ���ߵ�ƽ
  {
    retry++;
    delay_us(1);
  }
  delay_us(40); // �ȴ�40us
  if (GPIO_ReadInputDataBit(GPIO_DHT11, IO_DHT11) == 1)
    return 1;
  else
    return 0;
}

// ��DHT11��ȡһ���ֽ�
// ����ֵ������������
uint8_t DHT11_Read_Byte(void)
{
  uint8_t i, dat;
  dat = 0;
  for (i = 0; i < 8; i++)
  {
    dat <<= 1;
    dat |= DHT11_Read_Bit();
  }
  return dat;
}

// ��DHT11��ȡһ������
// ����ֵ��0,����;1,��ȡʧ��
uint8_t dht11_data[4]; // �����ʪ�����飬[0]-ʪ������ [1]-ʪ��С�� [2]-�¶����� [3]-�¶�С��
uint8_t DHT11_Update_Data(void)
{
  uint8_t buf[5] = {0};
  uint8_t i;
  DHT11_Rst();
  if (DHT11_Check() == 0)
  {
    for (i = 0; i < 5; i++) // ��ȡ40λ����
      buf[i] = DHT11_Read_Byte();
    if ((buf[0] + buf[1] + buf[2] + buf[3]) == buf[4])
      for (i = 0; i < 4; i++)
        dht11_data[i] = buf[i];
  }
  else
    return 1;
  return 0;
}

// ��ʼ��DHT11��IO�� DQ ͬʱ���DHT11�Ĵ���
// ����1:������
// ����0:����

void DHT11_Init(void)
{
  DHT11_Rst();   // ��λDHT11
  DHT11_Check(); // �ȴ�DHT11�Ļ�Ӧ
}
