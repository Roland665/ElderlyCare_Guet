#include <Arduino.h>
#include "MP3FileWriter.h"
#include "sd_card.h"

/**
 * @brief    创建默认格式的MP3编写器
 * @param    _sdcard: sd卡读写器
 * @retval   void
 */
MP3FileWriter::MP3FileWriter(sd_card *_sdcard)
{
  /* 编写ID3V2 */
  m_ID3V2.header.flag = 0; // 默认标志字节
  m_ID3V2_size = (m_ID3V2.header.size[0] & 0x7F) * 0x200000 + (m_ID3V2.header.size[1] & 0x7F) * 0x400 + (m_ID3V2.header.size[2] & 0x7F) * 0x80 + (m_ID3V2.header.size[3] & 0x7F);
  /* 编写ID3V1 */
  strcpy(m_ID3V1.artist, "Roland"); // 作者
  m_ID3V1.genre = 101;              // 音频类型：101=”Speech”
  m_ID3V1.reserve = 1;              // 无音轨号
  m_sdcard = _sdcard;
}

/**
 * @brief    开始向wav文件写入音频数据，并率先写入文件头
 * @param    void
 * @retval   0-成功打开wav文件
 */
int8_t MP3FileWriter::start(const char *MP3Path)
{
  ESP_LOGI("", "Start writing MP3 file");
  m_MP3Path = MP3Path;
  // 打开文件
  if (m_sdcard->start(m_MP3Path, "w"))
  {
    ESP_LOGE("", "Failed to open file for writing");
    return 1;
  }
  /* 写入 ID3V2 标签头 */
  m_sdcard->write(&m_ID3V2.header, sizeof(ID3V2_Header_t), 1);
  /* 写入 ID3V2 标签帧 */
  m_sdcard->write(m_ID3V2.frames, m_ID3V2_size, 1);
  return 0;
}


/**
 * @brief    向 MP3 文件写入音频帧
 * @param    samples    : 待写入的采样数据
 * @param    sample_size: 单采样数据的大小(一般用sizeof(数据类型)获取大小后传入)
 * @param    count      : 一次需写入的数量
 * @retval   void
 */
int8_t write(){
  uint32_t frameHead = 0;
  frameHead |= 0x000007FF; // 同步信息
  frameHead |= 0x000007FF; // 版本
}
