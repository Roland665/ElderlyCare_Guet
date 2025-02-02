// #include "mylist.h"
// /*********************************以下为对来自终端信息流处理函数*********************************/
// //创建结点
// terminal_net_info* Createterminal_net_infoNode(uint8_t* SLAddr, uint8_t type, uint8_t len, uint8_t* Data){
// 	terminal_net_info* newNode = (terminal_net_info*)malloc(sizeof(terminal_net_info));
// 	ArrCpy(8,newNode->SLAddr, SLAddr);
// 	newNode->type = type;
// 	newNode->len = len;
// 	ArrCpy(len,newNode->Data, Data);
// 	newNode->next = NULL;
// 	return newNode;
// }

// //创建链表
// terminal_net_info* Createterminal_net_infoList(void) {
// 	//表头结点
// 	terminal_net_info* headNode = (terminal_net_info*)malloc(sizeof(terminal_net_info));
// 	headNode->next = NULL;
// 	return headNode;
// }

// //末端插入法
// void Insterminal_net_infoNodeByEnd(terminal_net_info* headNode,uint8_t* SLAddr,uint8_t type, uint8_t len, uint8_t* Data) {
// 	terminal_net_info* posNode = headNode;
// 	if(posNode->next == NULL){
// 		terminal_net_info* newNode = Createterminal_net_infoNode(SLAddr,type, len, Data);
// 		posNode->next = newNode;
// 		return;
// 	}
// 	while (posNode->next != NULL){
// 		if(ArrCmp(len,posNode->next->Data,Data) == 1 && posNode->next->len == len && posNode->next->type == type && ArrCmp(8,posNode->next->SLAddr,SLAddr) == 1){
// 			//如果同一指令重复接收那就只接收第一次
// 			return;
// 		}
// 		posNode = posNode->next;

// 	}//定位在链表的最后一个节点
// 	terminal_net_info* newNode = Createterminal_net_infoNode(SLAddr,type, len, Data);
// 	posNode->next = newNode;//接上
// }

// //处理与终端间通信的数据流
// void Handleterminal_net_info(terminal_net_info* headNode){
// 	uint8_t Ack[] = {'O','K'};
// 	uint8_t AllowAck[] = {0x00,0x01};//同意终端入网有效数据
// 	uint8_t RefuseAck[] = {0x00,0x00};//拒绝终端入网有效数据
// 	uint8_t i;
// 	uint8_t Data[12];
// 	terminal_net_info* posNode = headNode;//执行兵，站在头结点
// 	terminal_net_info* posNodeFront;//后备兵,还没出场
// 	if(headNode->next == NULL) return;//如果链表没有数据流，那就直接退出函数了
// 	else{
// 		posNodeFront = headNode;//后备兵出场
// 		posNode = headNode->next;//执行兵走向下一条命令
// 	}
// 	while(posNode != NULL){//如果当前命令有内容，就执行
// 		//处理数据
// 		if(posNode->type == 0x00){//设备信息命令
// 			if(CheckDeviceNodeByLongAddr(DeviceList,posNode->SLAddr,&posNode->Data[1]) == 1){
// 				//如果链表内已经有该终端，更新下数据
// 				//设置透传目标为对应设备
// 				Zigbee_Change_Mode(0);
// 				Set_Send_Target(&posNode->Data[1],0x01);
// 				Zigbee_Change_Mode(1);
// 				Send_Custom_Data(USART1,0xFF,2,Ack);//回应下
// 				Send_Custom_Data(USART1,0xFF,2,Ack);//回应下
// 				Send_Custom_Data(USART1,0xFF,2,Ack);//回应下
// 				Send_Custom_Data(USART1,0xFF,2,AllowAck);//回应下
// 				Send_Custom_Data(USART1,0xFF,2,AllowAck);//回应下
// 				Send_Custom_Data(USART1,0xFF,2,AllowAck);//回应下
// 				APPOpenNetCountDown = 0;
// 			}
// 			else if(APPOpenNetCountDown > 0){
// 				//如果还在智能终端允许入网倒计时内,执行以下语句
// 				//重新封装该设备信息
// 				Data[0] = posNode->Data[0];
// 				Data[1] = 1;
// 				for(i = 0; i < 8; i++){
// 					Data[2+i] = posNode->SLAddr[i];
// 				}
// 				Data[10] = posNode->Data[1];
// 				Data[11] = posNode->Data[2];
// 				//等待APP的同意或拒绝信号
// 				APPJudgeFlag = 0;
// 				WaitTime = 0;
// 				while(APPJudgeFlag == 0){//等待APP的回应，有1分钟时间
// 					if(WaitTime >= 120){
// 						break;//退出循环，表示APP回应超时APPJudgeFlag仍为0
// 					}
// 					//将设备信息发给APP
// 					Esp32AckFlag = 0;
// //					while(Esp32AckFlag != 1){
// 						Send_Custom_Data(USART2,0x00,12,Data);
// 						delay_ms(3000);//稍微等等
// //					}
// 					//delay_ms(2700);
// 				}
// 				if(APPJudgeFlag == 1){//表示APP已同意
// 					//设置透传目标为对应设备
// 					Zigbee_Change_Mode(0);
// 					Set_Send_Target(&posNode->Data[1],0x01);
// 					Zigbee_Change_Mode(1);
// 					Send_Custom_Data(USART1,0xFF,2,AllowAck);//同意
// 					Send_Custom_Data(USART1,0xFF,2,AllowAck);//同意
// 					Send_Custom_Data(USART1,0xFF,2,AllowAck);//同意
// 					//纳入链表t
// 					InsertDeviceNodeByType(DeviceList,Data[0],1,posNode->SLAddr,&posNode->Data[1],0,NULL,0);
// 					AT24CXX_Save_List(0,DeviceList,SceneList);//即时存入EEPROM
// 				}
// 				else if(APPJudgeFlag == 2){//表示APP已拒绝
// 					//设置透传目标为对应设备
// 					Zigbee_Change_Mode(0);
// 					Set_Send_Target(&posNode->Data[1],0x01);
// 					Zigbee_Change_Mode(1);
// 					Send_Custom_Data(USART1,0xFF,2,RefuseAck);//拒绝
// 				}
// 			}
// 		}
// 		else if(posNode->type == 0x03){
// 			//如果是窗帘的返回数据->打开程度
// 			Send_Custom_Data(USART1,0xFF,2,Ack);//先回应再做自己的事
// 			uint8_t* CurtainDeep = OperateDeviceSelfDataByLAddr(DeviceList,posNode->SLAddr);
// 			ArrCpy(1,CurtainDeep,posNode->Data);
// 		}
// 		else if(posNode->type == 0x04){
// 			//空调专用传感器数据
// 			if(CheckDeviceNodeByLongAddr(DeviceList,posNode->SLAddr,SelfShortAddr) == 1){
// 				//如果数据源在生态内
// 				Send_Custom_Data(USART2,0x04,posNode->len,posNode->Data);//把温湿度数据发送到APP
// 			}
// 		}
// 		else if(posNode->type == 0x05){
// 			//独立温度传感器数据
// 			if(CheckDeviceNodeByLongAddr(DeviceList,posNode->SLAddr,SelfShortAddr) == 1){
// 				//如果数据源在生态内
// 				Send_Custom_Data(USART2,0x05,posNode->len,posNode->Data);//把温度数据发送到APP
// 				UpdateSensingData(SensingDataList,&Data[0],0x05,posNode->len,posNode->Data);
// 			}
// 		}
// 		else if(posNode->type == 0x08){
// 			//门锁返回数据(这里强调一下，门锁终端的SelfData[16]是代表着最新的数据所出的位置
// 			/*
// 			由于开的空间问题，这里只有3个位置，1~3，
// 			//如果SelfData[15] = 1,那么最新的数据在第一位；如果 = 2，那么是在第二位；如果 = 3，那么是在第三位；如果 = 0，那么是还没有数据）
// 			*/
// 			uint8_t* operateTime = OperateDeviceSelfDataByLAddr(DeviceList,posNode->SLAddr);//把对应终端的SelfData数组拿出来操作
// 			switch (operateTime[18])
// 			{
// 			case 0:
// 				i = 0;
// 				operateTime[18] = 1;
// 				break;
// 			case 1:
// 				i = 5;
// 				operateTime[18] = 2;
// 				break;
// 			case 2:
// 				i = 12;
// 				operateTime[18] = 3;
// 				break;
// 			case 3:
// 				i = 0;
// 				operateTime[18] = 1;
// 				break;
// 			default:
// 				break;
// 			}
// 			operateTime[i] = posNode->Data[0];
// 			i++;
// 			operateTime[i] = BJTimeInSecond / 3600;
// 			i++;
// 			operateTime[i] = BJTimeInSecond % 3600 / 60;
// 			i++;
// 			operateTime[i] = Date[0];
// 			i++;
// 			operateTime[i] = Date[1];
// 			i++;
// 			operateTime[i] = Date[2];
// 			i++;
// 		}
// 		//删除该结点
// 		posNodeFront->next = posNode->next;//后备兵的下一步指向执行兵的下一步
// 		free(posNode);//执行兵原地蒸发
// 		posNode = posNodeFront->next;//执行兵在后备兵下一步复活
// 	    HandleEsp32CommandStream(Esp32CommandStreamList);//处理完一个终端的事情，就处理与Esp32间通信的数据流
// 	}
// 	//链表空了
// }
// /***************************************END***************************************/
