#include "config.h"
#include "driver/SimpleBeep.h"
#include "driver/UART.h"
#include "driver/SimpleKey.h"
#include "driver/SimpleAsk.h"
#include "driver/SimpleLed.h"

#define LED P55

#define DEBUG_MAIN(BYTE) TX1_write2buff(BYTE)
// #define DEBUG_MAIN(BYTE)

bit read_key0_value()
{
    return P32;
}
const SimpleKey key0 = {read_key0_value, 0, 0, 0, 0, 0};

void write_beep_value(const u8 value)
{
    P10 = value;
}

const SimpleBeep beep = {write_beep_value, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void write_led_value(const u8 value)
{
    P16 = value;
}

const SimpleLed simpleLed = {write_led_value, 0};


#define ASK_RECEIVER_PIN P11 //接收端口
#define ASK_CCAPL CCAP0L     //接收端口对应的CCAPnL寄存器 低位
#define ASK_CCAPH CCAP0H     //接收端口对应的CCAPnH寄存器 高位
#define ASK_CCAPM CCAPM0     //接收端口对应的CCAPMn寄存器 模式位
#define ASK_CCF CCF0         //接收端口对应的PAC中断请求标志位

// #define ASK_RECEIVER_PIN P33 //接收端口
// #define ASK_CCAPL CCAP1L     //接收端口对应的CCAPnL寄存器 低位
// #define ASK_CCAPH CCAP1H     //接收端口对应的CCAPnH寄存器 高位
// #define ASK_CCAPM CCAPM1     //接收端口对应的CCAPMn寄存器 模式位
// #define ASK_CCF CCF1         //接收端口对应的PAC中断请求标志位

u16 read_ask_capture_counter_value()
{
    static u16 counter = 0;
    ((u8 *)&counter)[0] = ASK_CCAPH;
    ((u8 *)&counter)[1] = ASK_CCAPL;
    return counter;
}

bit read_ask_capture_ping_value()
{
    return ASK_RECEIVER_PIN;
}

SimpleAskReceiver askReceiver = {read_ask_capture_counter_value, read_ask_capture_ping_value, 0};

void PCA_Interrupt() interrupt 7
{
    DEBUG_MAIN(0x11);
    DEBUG_MAIN(ASK_RECEIVER_PIN);
    LED=ASK_RECEIVER_PIN;
    if (ASK_CCF)
    {
        ASK_CCF = 0;
        CF = 0;
        SimpleAskReceiver_HeartBeat(&askReceiver);
    }
}

void Timer0Init(void) // 1毫秒@12.000MHz
{
    AUXR &= 0x7F; //定时器时钟12T模式
    TMOD &= 0xF0; //设置定时器模式
    TL0 = 0x18;   //设置定时初始值
    TH0 = 0xFC;   //设置定时初始值
    TF0 = 0;      //清除TF0标志
    ET0 = 1;      //开定时器0中断
    TR0 = 1;      //定时器0开始计时
}


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

void Timer0_ISR_Handler(void) interrupt 1
{
    // LOGB(0x99);
    if (askReceiver.busy_counter > 0)
    {
        askReceiver.busy_counter--;
    }
    if (askReceiver.repeat_counter > 0)
    {
        askReceiver.repeat_counter--;
    }

    // 1ms
    SimpleKey_HeartBeat(&key0);
    SimpleBeep_HeartBeat(&beep);
    SimpleLed_HeartBeat(&simpleLed);
}



void delay_ms(unsigned char ms)
{
    unsigned int i;
    do
    {
        i = MAIN_Fosc / 10000;
        while (--i)
            ;
    } while (--ms);
}

#define LVDF 0x20
#define LVD2V0 0x00
#define LVD2V4 0x01
#define LVD2V7 0x02
#define LVD3V0 0x03
void Lvd_Isr() interrupt 6
{
    PCON &= ~LVDF;
    LOGB(0xBB);
}

u8 buffer_receive[ASK_FRAME_SIZE];
u8 read;
u8 i;

void main()
{
    // unsigned int i;
    P1_MODE_IO_PU(GPIO_Pin_6); // P1.6 LED灯
    P16=0;

    // P1_MODE_IN_HIZ(GPIO_Pin_1); // P1.1 设置为高阻输入 ASK接收引脚
    P3_MODE_IN_HIZ(GPIO_Pin_3); // P1.1 设置为高阻输入 ASK接收引脚
    P1_MODE_OUT_PP(GPIO_Pin_0); // P1.0 蜂鸣器引脚
    // P1_MODE_IO_PU(GPIO_Pin_0);
    // P1PU |= 0x01; // P1.0 打开上拉

    P3_MODE_IO_PU(GPIO_Pin_2); // key0
    P32 = 1;

    PCON &= ~LVDF;
    RSTCFG = LVD2V0;
    ELVD = 1;

    P1_MODE_IN_HIZ(GPIO_Pin_4); // P1.4 设置为高阻输入 USB电源引脚

    // CCON = 0x00;
    // CMOD = 0x08;
    // CL = 0x00;
    // CH = 0x00;

    // CCAPM1 = 0x42;
    // PCA_PWM1 = 0xc0;
    // CCAP1L = 0x80;
    // CCAP1H = 0x80;

    // CR =1;

    P10 = 0;
    UART_config();
    SimpleAskReceiver_Init(&askReceiver, 8,255, 400,1200);
    CCON = 0x00;
    CMOD = 0x00;
    CL = 0x00;
    CH = 0x00;
    ASK_CCAPL = 0x00;
    ASK_CCAPH = 0x00;
    ASK_CCAPM = 0x31; //上升沿及下降沿触发捕获
    

    Timer0Init();
    EA = 1;
    CR = 1;           //启动ASK PCA捕获
    LOGB(0XAA);
    LOGB(0XAA);
    LOGB(0XAA);

    // SimpleBeep_Start(&beep, BEEP_POWER_LESS_ME, 5, 255, 0);
    // SimpleBeep_Start(&beep, BEEP_POWER_LESS_OTHER, 5, 255, 0);
    SimpleBeep_Start(&beep, BEEP_MISSING, 5, 255, 0);
    
    // SimpleLed_Start(&simpleLed, 5, 5, 0);

    while (1)
    {

        if (SimpleKey_IsLongPressed(&key0))
        {
            // SimpleKey_Consumed(&key0);
            LOGB(P14);
            // LOGB(0xBB);
            // LOGB(0xBB);
        }

        // if (SimpleKey_IsClicked(&key0))
        // {
        //     SimpleKey_Consumed(&key0);
        //     // LOGB(0xCC);
        //     // LOGB(0xCC);
        //     // LOGB(0xCC);
        // }

        // read = SimpleAskReceiver_Read(&askReceiver,buffer_receive, ASK_FRAME_SIZE);
        // if (read == ASK_FRAME_SIZE)
        // {
        //     for (i = 0; i < read; i++)
        //     {
        //         LOGB(buffer_receive[i]);
        //     }
        // }
    }
}