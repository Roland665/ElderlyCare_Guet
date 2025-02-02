#include "myserver.h"
#include "Zigbee.h"

API_Server::API_Server(){
  terminal_list = createList();
  uint8_t SAddr[2];
  SAddr[0] = 0x1E;
  SAddr[1] = 0xFC;
  headInsertList(terminal_list, 1, SAddr, "我的小太阳");
  SAddr[0] = 0xCE;
  SAddr[1] = 0x9A;
  headInsertList(terminal_list, 1, SAddr, "卧室小月亮");
  SAddr[0] = 0xB0;
  SAddr[1] = 0x4A;
  headInsertList(terminal_list, 1, SAddr, "厕所的灯");
  SAddr[0] = 0x68;
  SAddr[1] = 0xAF;
  headInsertList(terminal_list, 2, SAddr, "世界之窗");
  m_zigbee_buffer[0] = 6;
  m_zigbee_buffer[1] = 6;
  m_zigbee_buffer[2] = 5;
}

list_node_t *API_Server::get_terminal_list(void){
  // clearList(terminal_list);
  // terminal_list = createList();
  return terminal_list;
}


/* 分析语音控制命令函数 */
void API_Server::analyse_command(const char * command, Zigbee *zigbee){
  list_node_t *move = terminal_list->next;
  uint8_t i = 3;
  while(move){
    ESP_LOGD("", "正在从 %s 中寻找 %s", command, move->DName);
    if(strstr(command, move->DName)){
      m_zigbee_buffer[i++] = move->SAddr[0];
      m_zigbee_buffer[i++] = move->SAddr[1];
      if(move->DCode == 0x01){ // 如果是对灯的控制
        m_zigbee_buffer[i++] = 0x01;
        m_zigbee_buffer[i++] = 3;
        if(strstr(command, "第一") || strstr(command, "第1"))
          m_zigbee_buffer[i++] = 0x01;
        else if(strstr(command, "第二") || strstr(command, "第2"))
          m_zigbee_buffer[i++] = 0x02;
        else if(strstr(command, "第三") || strstr(command, "第3"))
          m_zigbee_buffer[i++] = 0x03;
        else if(strstr(command, "第四") || strstr(command, "第4"))
          m_zigbee_buffer[i++] = 0x04;
        else
          m_zigbee_buffer[i++] = 0x00;

        m_zigbee_buffer[i++] = 0x00; // 调节亮度
        char *bright_str = strstr(command, "%");
        if(bright_str){
          bright_str-=2;
          if(bright_str[0] <= '9' && bright_str[0] >= '0')
            m_zigbee_buffer[i++] = bright_str[0]*10+bright_str[1];
          else
            m_zigbee_buffer[i++] = bright_str[1];
        }
        else{ // 如果没有指定亮度，则默认开就是开到80，关就是全关
          if(strstr(command, "关"))
            m_zigbee_buffer[i++] = 0;
          else
            m_zigbee_buffer[i++] = 80;
        }
      }
      else if(move->DCode == 0x02){ // 如果是对窗帘控制
        m_zigbee_buffer[i++] = 0x02;
        m_zigbee_buffer[i++] = 2;
        if(strstr(command, "左"))
          m_zigbee_buffer[i++] = 0x01;
        else if(strstr(command, "右"))
          m_zigbee_buffer[i++] = 0x02;
        else
          m_zigbee_buffer[i++] = 0x00;

        char *open_str = strstr(command, "%");
        if(open_str){
          open_str-=2;
          if(open_str[0] <= '9' && open_str[0] >= '0')
            m_zigbee_buffer[i++] = open_str[0]*10+open_str[1];
          else
            m_zigbee_buffer[i++] = open_str[1];
        }
        else{ // 如果没有指定程度，则默认开就是开到100%，关就是全关
          if(strstr(command, "关"))
            m_zigbee_buffer[i++] = 0;
          else
            m_zigbee_buffer[i++] = 100;
        }
      }
      zigbee->change_mode(1);
      zigbee->send(m_zigbee_buffer, i);
      ESP_LOGI("", "Success control %s", move->DName);
      return;
    }
    move = move->next;
  }
}

API_Server::~API_Server(){
}

