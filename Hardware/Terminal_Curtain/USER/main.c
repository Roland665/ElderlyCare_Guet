#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include <stdlib.h>

#include "usart.h"
#include "led/led.h"
#include "key/key.h"
#include "Zigbee/Zigbee.h"
#include "timer/timer.h"
#include "delay.h"
#include "DHT11/DHT11.h"
/*
@version v1.0
窗帘控制终端
*/

typedef struct _Command {
    uint8_t *buffer;
    uint32_t length;
} Command;

Zigbee zigbee = {
    .modeFlag           = 2,
    .readySetTargetFlag = 1,
    .zigbeeOnlineFlag   = 0,
    .terminalOnlineFlag = 0};

/******************************* 宏定义 ************************************/
/*最多可以存储 10 个 u8类型变量的队列 */
#define CFRAME_QUEUE_LENGTH 10
#define CFRAME_ITEM_SIZE    sizeof(Command)

/********************************** 内核对象 *********************************/
/* 无线控制命令消息队列句柄 */
static QueueHandle_t cFrame_queue = NULL;
/* 发送设备信息标志消息队列句柄*/
static SemaphoreHandle_t infoFlag_semaphore = NULL;

/******************************* 任务对象 ************************************/
/* 空闲任务任务堆栈 */
static StackType_t Idle_Task_Stack[configMINIMAL_STACK_SIZE];
/* 空闲任务控制块 */
static StaticTask_t Idle_Task_TCB;
/* 定时器任务堆栈 */
static StackType_t Timer_Task_Stack[configTIMER_TASK_STACK_DEPTH];
/* 定时器任务控制块 */
static StaticTask_t Timer_Task_TCB;

///* 心跳包任务栈深 */
// #define SendHBPack_TASK_STACK_DEPTH 128*1
///* 心跳包任务堆栈 */
// static StackType_t SendHBPack_Task_Stack[SendHBPack_TASK_STACK_DEPTH];
///* 心跳包任务控制块 */
// static StaticTask_t SendHBPack_Task_TCB;

/**
 * @brief    获取空闲任务的任务堆栈和任务控制块内存
 * @param    ppxIdleTaskTCBBuffer		:	任务控制块内存
 * @param    ppxIdleTaskStackBuffer		:	任务堆栈内存
 * @param    pulIdleTaskStackSize		:	任务堆栈大小
 * @retval   void
 */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize)
{
    *ppxIdleTaskTCBBuffer   = &Idle_Task_TCB;           /* 任务控制块内存 */
    *ppxIdleTaskStackBuffer = Idle_Task_Stack;          /* 任务堆栈内存 */
    *pulIdleTaskStackSize   = configMINIMAL_STACK_SIZE; /* 任务堆栈大小 */
}

/**
 * @brief    获取定时器任务的任务堆栈和任务控制块内存
 * @param    ppxTimerTaskTCBBuffer	:		任务控制块内存
 * @param    ppxTimerTaskStackBuffer:		任务堆栈内存
 * @param    pulTimerTaskStackSize	:		任务堆栈大小
 * @retval   void
 */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize)
{
    *ppxTimerTaskTCBBuffer   = &Timer_Task_TCB;              /* 任务控制块内存 */
    *ppxTimerTaskStackBuffer = Timer_Task_Stack;             /* 任务堆栈内存 */
    *pulTimerTaskStackSize   = configTIMER_TASK_STACK_DEPTH; /* 任务堆栈大小 */
}

/******************************* 普通全局变量 ************************************/
uint8_t APPJudgeFlag = 0; // 来自APP的入网判断标志位，如果为1，表示同意，为2表示拒绝，闲时置0

void Try_To_Link(Zigbee *zigbee)
{
    reset_LED1;
    Zigbee_Change_Mode(zigbee, 0);                                             // Zigbee进入HEX指令模式
    if (zigbee->zigbeeOnlineFlag == 1) Zigbee_Restore_Factory_Setting(zigbee); // Zigbee恢复出厂设置
    delay(100);
    Zigbee_Restart(zigbee);
    delay(1000);
    Zigbee_Set_Type_To_Active_Terminal(zigbee); // 设置模组类型为活跃终端
    delay(100);
    Zigbee_Restart(zigbee);
    delay(100);
    Zigbee_Open_Net(zigbee); // 打开网络准备配对
    delay(7000);             // 稍微等待联网
    zigbee->zigbeeOnlineFlag = 0;
    uint8_t waitCount        = 0;
    Zigbee_Get_State(zigbee); // 获取设备状态
    while (zigbee->zigbeeOnlineFlag != 1) {
        if (waitCount >= 200) { // 超过100sZigbee没有连上协调器,判断为中控没开机，直接退出while
            break;
        }
        Zigbee_Get_State(zigbee); // 获取设备状态
        reset_LED1;
        delay(100);
        set_LED1;
        delay(400);
        waitCount++;
    }
    if (zigbee->zigbeeOnlineFlag == 1) {
        // 如果已经连上了协调器
        LED1FlashTime = 120; // 闪烁两分钟,表示正在配对中
        Zigbee_Restart(zigbee);
        Zigbee_Set_Send_Target(zigbee); // 将透传目标改为协调器
        Zigbee_Change_Mode(zigbee, 1);  // 进入透传模式

        // 等待APP同意或拒绝
        APPJudgeFlag               = 0;
        waitCount                  = 0;
        zigbee->terminalOnlineFlag = 0;
        while (APPJudgeFlag == 0) {
            if (waitCount >= 100) // 100s没收到应答，直接退出，表示为没有入网
                return;
            xSemaphoreGive(infoFlag_semaphore);
            delay(1000); // 考虑数据接收延迟,避免频繁发送导致中控数据拥堵
            waitCount++;
        }
        if (APPJudgeFlag == 1)
            zigbee->terminalOnlineFlag = 1; // 入网成功
        else if (APPJudgeFlag == 2)
            zigbee->terminalOnlineFlag = 0; // 入网失败
    }
    LED1FlashTime = 0; // 指示灯停止闪烁
    set_LED1;          // 熄灯
}

/**
 * @brief    Key_Task 任务主体
 * @param    parameter
 * @retval   void
 */
static void Key_Task(void *parameter)
{
    uint8_t key = 0;
    while (1) {
        key = key_scan();
        switch (key) {
            case 0:
                net_key_time = 0;
                while (net_key == 0 || (net_key == 1 && net_KeyCD < KEYCD_TIME)) {
                    if (net_key_time >= 3)
                        Try_To_Link(&zigbee);
                }
                break;
        }
    }
}

// 心跳包发送任务
static void SendHBPack_Task(void *parameter)
{
    uint8_t i;
    /* 初始化DHT11 */
    DHT11_Init();
    DHT11_Update_Data();
    Zigbee *zigbee = parameter;
    Zigbee_Change_Mode(zigbee, 0);
    Zigbee_Get_State(zigbee);
    if (zigbee->zigbeeOnlineFlag == 1) {
        Zigbee_Set_Send_Target(zigbee);
        Zigbee_Change_Mode(zigbee, 1);
        Zigbee_Update_TerminalOnlineFlag(zigbee);
    }
    uint8_t HBPack[1 + 2 + 8 + 1];
    while (1) {
        if (zigbee->zigbeeOnlineFlag) {
            DHT11_Update_Data(); // 更新传感器数据
            if (zigbee->terminalOnlineFlag) {
                reset_LED1;
                // 每秒发送一次心跳包
                delay(1000);
                i           = 0;
                HBPack[i++] = 0x02; // 这里一定要修改为对应终端的设备类型码
                HBPack[i++] = zigbee->SAddr[0];
                HBPack[i++] = zigbee->SAddr[1];
                HBPack[i++] = 0; // 无需应答
                for (uint8_t j = 0; j < 2; j++)
                    HBPack[i++] = PWMval[j];
                for (uint8_t j = 0; j < 4; j++)
                    HBPack[i++] = dht11_data[j];
                send_customFrame(zigbee, 0xFE, 1 + 2 + 1 + 6, HBPack);
            } else {
                set_LED1;
                // 每30s确认一次中控在线状态，如果期间需要发送自身设备信息则直接发送
                if (xSemaphoreTake(infoFlag_semaphore, 30000) == pdPASS) {
                    // 如果有入网申请需要发送
                    HBPack[0] = 0x02; // 这里一定要修改为对应终端的设备类型码
                    HBPack[1] = zigbee->SAddr[0];
                    HBPack[2] = zigbee->SAddr[1];
                    send_customFrame(zigbee, 0x00, 3, HBPack); // 发送入网申请
                } else {
                    Zigbee_Update_TerminalOnlineFlag(zigbee);
                }
            }
        } else {
            reset_LED1;
            delay(1000);
        }
    }
}

// 分析控制命令任务
static void AnalyseCommand_Task(void *parameter)
{
    Zigbee *zigbee = parameter;
    Command command;
    uint8_t ack[] = {0x4F, 0x4B};
    while (1) {
        // 等待中控的消息
        xQueueReceive(cFrame_queue, &command, portMAX_DELAY);
        if (command.buffer[0] == 0x55) // Zigbee反馈命令
            Zigbee_Analyse_Command_Data(zigbee);
        else if (command.buffer[0] == '6' && command.buffer[1] == '6' && command.buffer[2] == '5') {
            send_customFrame(zigbee, 0xFF, 2, ack);
            if (command.buffer[3] == 0x02) { // 窗帘调节 【这里一定要修改为对应终端的设备类型码】
              PWM_Curtain_Set(command.buffer[5], command.buffer[6]);
            } else if (command.buffer[3] == 0x00) { // 入网请求反馈
                if (command.buffer[5] == 0x01)
                    zigbee->terminalOnlineFlag = 1;
            } else if (command.buffer[3] == 0xFF) { // 中控应答
                if (command.buffer[5] == 0x4F && command.buffer[6] == 0x4B)
                    zigbee->ackFlag = 1;
            }
        }
        free(command.buffer);
    }
}

// 板载外设初始化
static void setup(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); // 4bit都用于设置抢占优先级
    LED_Init();                                     // 初始化所有指示用LED
    reset_LED2;
    key_init(); // 初始化所有按键
    usart1_init(115200);
    TIM2_Int_Init(1000 - 1, 72 - 1); // 1ms中断一次
    TIM3_PWM_Init(20000 - 1, 72 - 1);  // 72M/72=1Mhz 的计数频率,重装载值 20000 ，所以PWM频率为 1M/20000=50hz.(周期为20ms)
}

int main(void)
{
    setup(); // 初始化
    cFrame_queue       = xQueueCreate(CFRAME_QUEUE_LENGTH, CFRAME_ITEM_SIZE);
    infoFlag_semaphore = xSemaphoreCreateBinary();
    /* 创建任务 */
    /* Key_Task 任务 */
    // LED_Task_Handle = xTaskCreate(Key_Task, "Key_Task", 128, NULL, 4, LED_Task_Stack, &LED_Task_TCB);
    xTaskCreate(Key_Task, "Key_Task", 128, NULL, 3, NULL);

    /* SendHBPack_Task 任务 */
    // xTaskCreateStatic(SendHBPack_Task, "SendHBPack_Task", SendHBPack_TASK_STACK_DEPTH, &zigbee, 4, SendHBPack_Task_Stack, &SendHBPack_Task_TCB);
    xTaskCreate(SendHBPack_Task, "SendHBPack_Task", 128, &zigbee, 4, NULL);

    /* AnalyseCommand_Task 任务 */
    xTaskCreate(AnalyseCommand_Task, "AnalyseCommand_Task", 128, &zigbee, 5, NULL);

    /* 开启调度器 */
    vTaskStartScheduler();
    while (1)
        ;
}

/**
 * @brief		串口1中断服务程序
 * @param		void
 * @retval		void
 */
void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) { // 接收中断
        if (usart1_rx_len < UART_RX_LEN_MAX) {
            usart1RXTime                    = 0;
            usart1_rx_buffer[usart1_rx_len] = USART_ReceiveData(USART1); // 读取接收到的数据
            usart1_rx_len++;
        } else {
            usart1_rx_len = 0;
        }
    }
}

// 1ms级中断
void TIM2_IRQHandler(void)
{
    u8 i;
    static uint32_t millisecond = 0;
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET) { // 溢出中断
        // ms级MCU计时
        millisecond++;
        if (usart1RXTime < UART1_WAIT_TIME)
            usart1RXTime++;

        if (usart1RXTime == UART1_WAIT_TIME) {
            // 接收完了一串串口消息
            static Command command;
            command.buffer = (uint8_t *)malloc(usart1_rx_len);
            command.length = usart1_rx_len;
            for (i = 0; i < usart1_rx_len; i++) {
                command.buffer[i] = usart1_rx_buffer[i];
            }
            xQueueSendFromISR(cFrame_queue, &command, NULL);
            usart1_rx_len = 0;
            usart1RXTime  = 0xFF; // 把时间拉满，表示没有收到新的消息
        }

        // 按键消抖计时
        if (net_KeyCD < KEYCD_TIME) net_KeyCD++;

        if (millisecond == 1000) { // 以下语句每秒执行一次
            if (LED1FlashTime > 0) {
                LED1FlashTime--;
                if (LED1FlashTime == 0xFF) LED1FlashTime = 0;
                if (LED1FlashTime == 0) {
                    set_LED1;
                }
            }

            if (net_key_time < 0xFF) net_key_time++;
            millisecond = 0;
        }
    }
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update); // 清除中断标志位
}
