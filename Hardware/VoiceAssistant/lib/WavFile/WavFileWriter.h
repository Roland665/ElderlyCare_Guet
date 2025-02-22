#pragma once

#include "WavFile.h"
#include "sd_card.h"

class WavFileWriter
{

private:
  long m_file_size;      // Wave file size
  wav_header_t m_header; // Wave file header
  const char *m_wavPath;       // Wave file path
  sd_card *m_sdcard;     // sd卡操作对象，使用WavFileWriter的过程保证此对象存在

public:
  WavFileWriter(sd_card *_sdcard, const char *wavPath, int sample_rate, uint8_t bytes_per_sample);
  int8_t start();
  int8_t write(const uint8_t *samples, size_t sample_size, int count);
  int8_t finish();
};
