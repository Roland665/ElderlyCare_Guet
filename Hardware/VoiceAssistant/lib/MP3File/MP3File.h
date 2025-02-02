#pragma once
#include <stdint.h>

/* ID3V2标签头 */
typedef struct _ID3V2_Header_t
{
  char header[3]; // 必须为“ID3”否则认为标签不存在
  char ver;       // 版本号ID3V2.3 就记录3
  char revision;  // 副版本号此版本记录为0
  char flag;      // 标志字节，只使用高三位，其它位为0
  char size[4];   // 标签大小
  _ID3V2_Header_t()
  {
    header[0] = 'I';
    header[1] = 'D';
    header[2] = '3';
    ver = 3;
    revision = 0;
    size[0] = 0;
    size[1] = 0;
    size[2] = 0;
    size[3] = 0;
  }
} ID3V2_Header_t, *pID3V2_Header_t;
/* ID3V2标签帧 */
typedef struct _ID3V2_Frame_t
{
  char ID[4];    // 标识帧，说明其内容，例如作者/标题等
  char size[4];  // 帧内容的大小，不包括帧头，不得小于1
  char flags[2]; // 标志帧，只定义了6位
} ID3V2_Frame_t, *pID3V2_Frame_t;
/* ID3V2 */
typedef struct _ID3V2_t
{
  ID3V2_Header_t header; // 标签头
  pID3V2_Frame_t frames; // 标签帧
} ID3V2_t, *pID3V2_t;

/* ID3V1 */
typedef struct _ID3V1_t
{
  char header[3];         // 标签头必须是"TAG"否则认为没有标签
  char title[30] = {0};   // 标题
  char artist[30] = {0};  // 作者
  char album[30] = {0};   // 专集
  char year[4] = {0};     // 出品年代
  char comment[28] = {0}; // 备注
  char reserve;           // 保留位，0-表示无音轨号，非0-表示有音轨号
  char track;             // 音轨号，表示歌曲在专辑中的序号
  char genre;             // 音乐类型
  _ID3V1_t()
  {
    header[0] = 'T';
    header[1] = 'A';
    header[2] = 'G';
  }
} ID3V1_t, *pID3V1_t;
