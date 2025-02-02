#pragma once
#include "MP3File.h"

class MP3FileWriter
{
private:
  ID3V2_t m_ID3V2;
  uint32_t m_ID3V2_size;
  ID3V1_t m_ID3V1;
  uint32_t FrameHead; // 帧头
  sd_card *m_sdcard;  // sd卡操作对象，使用需保证此对象存在
  const char *m_MP3Path;    // MP3文件输出路径
public:
  MP3FileWriter(sd_card *_sdcard);
  int8_t start(const char *MP3Path);
};
