#pragma once
#include "API_online.h"
#include "myList.h"
#include "Zigbee.h"

class API_Server
{
private:
  String m_url;
  list_node_t *terminal_list;
  uint8_t m_zigbee_buffer[3+1+1+3];
public:
  API_Server(void);
  list_node_t *get_terminal_list(void);
  void analyse_command(const char *command, Zigbee *zigbee);
  ~API_Server();
};
