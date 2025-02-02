#include "Zigbee.h"

#define wait delay(100)

static char *TAG = "Zigbee";

Zigbee::Zigbee(HardwareSerial *serial):m_serial(serial){
  serial->begin(115200, SERIAL_8E1, ZIGBEE_RX, ZIGBEE_TX);
}

/**
  * @brief		打开Zigbee模块的网络（协调器：如果未建立网络，则建立一个新网络；如果已建立网络，则再调用此函数时为开放网络180s，180s内终端和路由器可加入）
  * @param		void
  * @retval		1->网络打开成功
  */

void Zigbee::openNet(){
	uint8_t netStart_command[] = {0x55, 0x04, 0x00, 0x02, 0x00, 0x02,};//开始配网:协调器执行该命令会打开入网允许权限,路由和终端节点执行该命令会尝试加入一个协调器创建的网络
	uint8_t i;
  ESP_LOGI(TAG, "Opening net");
	while(m_openNetFlag == 0){
		m_serial->write(netStart_command, 6);
    ESP_LOGI(TAG, ".");
		wait;
	}
	m_openNetFlag = 0;
}

/**
  * @brief		切换Zigbee模式
  * @param		mode -> 0为进入HEX指令模式，1为进入透传模式
  * @retval	  void
  */
void Zigbee::change_mode(uint8_t mode){
	uint8_t change_mode0_command[] = {0x2B, 0x2B, 0x2B};//Zigbee透传模式切换HEX指令模式 0为HEX指令模式
	uint8_t change_mode1_command[] = {0x55, 0x07, 0x00, 0x11, 0x00, 0x03, 0x00, 0x01, 0x13};//Zigbee HEX指令模式切换透传模式 1为数据透传模式
	uint8_t i;
	if(mode == 0){
		//进入配置模式
    ESP_LOGI(TAG, "Changing mode to mode0");
		while(m_modeFlag != 0){
			m_serial->write(change_mode0_command, 3);
      ESP_LOGI(TAG, ".");
			wait;
		}
    ESP_LOGI(TAG, "Zigbee module has entered mode 0");
	}
	else if(mode == 1){
		//进入透传模式
    ESP_LOGI(TAG, "Zigbee module has entered mode 1");
		while(m_modeFlag != 1){
			m_serial->write(change_mode1_command, 9);
      ESP_LOGI(TAG, ".");
      wait;
		}
    ESP_LOGI(TAG, "Zigbee module entered mode1");
	}
}

/**
  * @brief		查询模组当前状态（读取参数）
  * @param		void
  * @retval	    1->成功获取状态
  */

void Zigbee::get_state(void){
	uint8_t i;
	uint8_t getstate_command[] = {0x55, 0x03, 0x00, 0x00, 0x00};//查询Zigbee模组当前状态
  ESP_LOGI(TAG, "Checking zigbee model state");
	while(m_getStateFlag != 1){
		m_serial->write(getstate_command, 5);
    ESP_LOGI(TAG, ".");
    wait;
	}
	m_getStateFlag = 0;
}


/**
  * @brief		设置目标短地址和目标端口
  * @param		DSAddr:目标短地址
  * @param		DPort:目标端口
  * @retval		1->设置成功
  */

void Zigbee::set_send_target(uint8_t *DSAddr,uint8_t DPort){
	uint8_t setDirShortAddr_command[] = {0x55,0x08,0x00,0x11,0x00,0x01,0x00,DSAddr[0],DSAddr[1],0x00};
	uint8_t setport_command[] = {0x55,0x07,0x00,0x11,0x00,0x02,0x00,DPort,0x00};
	uint8_t temp = 0,i;
	//计算一下上面两个命令的校验码
	for(i = 0; i < setDirShortAddr_command[1] - 1; i++){
		temp = temp^setDirShortAddr_command[2+i];
	}
    setDirShortAddr_command[9] = temp;
	temp = 0;
	for(i = 0; i < setport_command[1] - 1; i++){
		temp = temp^setport_command[2+i];
	}
	setport_command[8] = temp;

  // 确保Zigbee进入配置模式
  if(m_modeFlag != 0)
    change_mode(0);

	// 设置目标短地址
	m_setTargetReadyFlag = 0; // 更新状态
  ESP_LOGI(TAG, "Setting zigbee sending direction short address: %02X %02X", DSAddr[0], DSAddr[1]);
	while(m_setTargetStep == 0){
		m_serial->write(setDirShortAddr_command, 10);
    ESP_LOGI(TAG, ".");
    wait;
	}
  ESP_LOGI(TAG, "Setting zigbee sending direction port");
	//设置目标端口
	while(m_setTargetStep == 1){
		m_serial->write(setport_command, 9);
    ESP_LOGI(TAG, ".");
    wait;
	}
	m_setTargetReadyFlag = 1;
}


/**
  * @brief		分析Zigbee的反馈命令
  * @param		void
  * @retval	void
  */
void Zigbee::analyze_feedback(uint8_t *buffer){
  ESP_LOGI(TAG, "Analyzing zigbee model feedback");
  if(buffer[1] == 0x2A && buffer[2] == 0x00 && buffer[3] == 0x00 && buffer[4] == 0x00){//55 2A 00 00 00
    // 查询状态返回数据
    m_longAddr[0] = buffer[6];
    m_longAddr[1] = buffer[7];
    m_longAddr[2] = buffer[8];
    m_longAddr[3] = buffer[9];
    m_longAddr[4] = buffer[10];
    m_longAddr[5] = buffer[11];
    m_longAddr[6] = buffer[12];
    m_longAddr[7] = buffer[13];
    m_shortAddr[0] = buffer[17];
    m_shortAddr[1] = buffer[18];
    m_getStateFlag = 1;
  }
  else if(buffer[1] == 0x04 && buffer[2] == 0x00 && buffer[3] == 0x02 && buffer[4] == 0x00 && buffer[5] == 0x02){//55 04 00 02 00 02
    m_openNetFlag = 1;//判断网络已打开
  }
  else if(m_setTargetReadyFlag == 1 && buffer[1] == 0x04 && buffer[2] == 0x00 && buffer[3] == 0x11 && buffer[4] == 0x00 && buffer[5] == 0x11){//55 04 00 11 00 11
    m_modeFlag = 1;
  }
  else if(m_setTargetReadyFlag == 0 && buffer[1] == 0x04 && buffer[2] == 0x00 && buffer[3] == 0x11 && buffer[4] == 0x00 && buffer[5] == 0x11){//55 04 00 11 00 11
    if(m_setTargetStep == 0) m_setTargetStep = 1;
    else m_setTargetStep = 0;
  }
  else if(buffer[1] == 0x03 && buffer[2] == 0xFF && buffer[3] == 0xFE && buffer[4] == 0x01){//55 03 FF FE 01
    if(m_modeFlag != 0) m_modeFlag = 0;
  }

}


void Zigbee::start(void){
  // 获取Zigbee的状态并确保进入传输模式
  change_mode(0);// 进入配置模式
  get_state();// 读取Zigbee模块信息
  change_mode(1);// 进入传输模式
}

// 若想透传数据，则使用此函数前一定要调用 zigbee_change_mode(1)；若是想配置Zigbee模块，则一定要调用 zigbee_change_mode(0)。
void Zigbee::send(uint8_t *buffer, uint32_t length){
  m_serial->write(buffer, length);
}

uint8_t Zigbee::get_modeFlag(void){
  return m_modeFlag;
}
