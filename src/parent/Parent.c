#include "../config.h"
#include "Task.h"
#include "App.h"
#include "ASK.h"
#include "UART.h"
#include "Key.h"

//========================================================================
//                                IO口配置
//========================================================================
void GPIO_config(void)
{

#ifdef STC8G1K08A
    P3_MODE_IN_HIZ(GPIO_Pin_3); // P3.3 设置为高阻输入 ASK接收引脚
    P3_MODE_IO_PU(GPIO_Pin_2); // P3.2 按键引脚
    P5_MODE_IO_PU(GPIO_Pin_5); // P5.5 LED灯

    // P3PU |= 0x04; // P3.2 打开上拉

#endif                          // STC8G1K08A

#ifdef STC8G1K17

    P1_MODE_IN_HIZ(GPIO_Pin_0); // P1.0 设置为高阻输入 ASK接收引脚
    P1_MODE_OUT_PP(GPIO_Pin_1); // P1.1 设置为推挽输出 为ASK接收模块供电
    P11 = 1;

    P3_MODE_OUT_OD(GPIO_Pin_6); // P3.6 设置为开漏输出 ASK发送引脚
    P3_MODE_OUT_PP(GPIO_Pin_7); // P3.7 设置为推挽输出 为ASK发送模块供电
    P37 = 1;

#endif // STC8G1K17
}

//========================================================================
//                               定时器配置
//========================================================================
void Timer_config(void)
{
    //[该定时器为系统运行心跳，必须的]
    // 50微秒@12.000MHz
    AUXR &= 0x7F; //定时器时钟12T模式
    TMOD &= 0xF0; //设置定时器模式
    TMOD |= 0x02; //设置定时器模式
    TL0 = 0xCE;   //设置定时初始值
    TH0 = 0xCE;   //设置定时重载值
    TF0 = 0;      //清除TF0标志
    TR0 = 1;      //定时器0开始计时

    ET0 = 1; //使能定时器中断
    EA = 1;
    TR0 = 1; //定时器0开始计时
}

//========================================================================
//                              串口初始化
//========================================================================
void UART_config(void)
{
    //串口初始化 模式3 不占用定时器的固定波特率模式 MAIN_Fosc/64   当12MHZ时候，波特率为【187500】
    COMx_InitDefine COMx_InitStructure;       //结构定义
    COMx_InitStructure.UART_Mode = UART_9bit; //模式,       UART_ShiftRight,UART_8bit_BRTx,UART_9bit,UART_9bit_BRTx
    // COMx_InitStructure.UART_BRT_Use   = BRT_Timer1;			//选择波特率发生器,   BRT_Timer1, BRT_Timer2 (注意: 串口2固定使用BRT_Timer2)
    //  COMx_InitStructure.UART_BaudRate  = 115200ul;			//波特率, 一般 110 ~ 115200
    COMx_InitStructure.UART_RxEnable = DISABLE;      //接收允许,   ENABLE或DISABLE
    COMx_InitStructure.BaudRateDouble = DISABLE;     //波特率加倍, ENABLE或DISABLE
    COMx_InitStructure.UART_Interrupt = ENABLE;      //中断允许,   ENABLE或DISABLE
    COMx_InitStructure.UART_Priority = Priority_0;   //指定中断优先级(低到高) Priority_0,Priority_1,Priority_2,Priority_3
    COMx_InitStructure.UART_P_SW = UART1_SW_P30_P31; //切换端口,   UART1_SW_P30_P31,UART1_SW_P36_P37,UART1_SW_P16_P17,UART1_SW_P43_P44
    UART_Configuration(UART1, &COMx_InitStructure);  //初始化串口1 UART1,UART2,UART3,UART4
}

u8 timer_1ms_counter = 0;
void Timer0_ISR_Handler(void) interrupt 1
{
#ifdef ASK_SENDER
    Ask_SenderProcess();
#endif

    if (++timer_1ms_counter > 19)
    { // 1ms触发一次
        timer_1ms_counter = 0;
        Task_Marks_Handler_Callback(); //任务标记回调函数
    }
}

//========================================================================
//                                系统初始化
//========================================================================

static TASK_COMPONENTS Task_Comps[]=
{
//状态  计数  周期  函数
	{0, 500, 500, App_Run},				/* task 1 Period： 500ms */
	{0, 10, 10, Key_Detect},				/* task 1 Period： 10ms */
};

void SYS_Init(void)
{
    GPIO_config();
    Key_Init();
    UART_config();
    Ask_ReceiverInit();
    App_Config();
    Task_Init(Task_Comps, sizeof(Task_Comps) / sizeof(TASK_COMPONENTS));
    Timer_config();
    EA = 1;
}


void main(void)
{
    SYS_Init();

    while (1)
    {
        Task_Pro_Handler_Callback();
    }
}
