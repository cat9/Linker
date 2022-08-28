#include "../config.h"
#include "../driver/UART.h"
#include "../driver/SimpleAsk.h"

#define LED P55

#define ASK_SENDER_POWER_SWITCH P54


void write_ask_send_pin(const u8 value)
{
    P33 = value;
}

const SimpleAskSender askSender={write_ask_send_pin,0};

char *DEV_ID;
u8 TEST_ID[7] = {0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61};

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

void delay_ms(unsigned char ms)
{
	unsigned int i;
	do{
		i = MAIN_Fosc / 10000;
		while(--i);
	}while(--ms);
}

//========================================================================
//                               定时器配置
//========================================================================
void Timer_config(void)
{
    // 50微秒@12.000MHz
    AUXR &= 0x7F; //定时器时钟12T模式
    TMOD &= 0xF0; //设置定时器模式
    TMOD |= 0x02; //设置定时器模式
    TL0 = 0xCE;   //设置定时初始值
    TH0 = 0xCE;   //设置定时重载值
    TF0 = 0;      //清除TF0标志
    // TR0 = 1;      //定时器0开始计时

    ET0 = 1; //使能定时器中断
    EA = 1;
    // TR0 = 1; //定时器0开始计时
}

void Timer0_ISR_Handler(void) interrupt 1
{
    SimpleAskSender_HeartBeat(&askSender);
}

void Child_Init(void){
    
    DEV_ID = (char code *)0x1ff9; //获取设备ID
    // DEV_ID = &TEST_ID;
    UART_config();
    Timer_config();
    SimpleAskSender_Init(&askSender,50,400,1200);

    P5_MODE_OUT_PP(GPIO_Pin_4); // P5.4 设置为推挽输出，为发射模块提供电源
    P3_MODE_IO_PU(GPIO_Pin_3); // P3.3 设置为标准IO，为发射模块数据端口
    P3_MODE_IO_PU(GPIO_Pin_2); // P3.2 设置为标准IO，为外部中断0输入

    P5_MODE_IO_PU(GPIO_Pin_5); // P5.5 LED灯

    //掉电唤醒时钟计数器32KHZ,约等于 16 秒
    WKTCH = 0x7F;
    WKTCL = 0xFE;

    IT0 =1;

    EA = 1;

}

void int0_interrupt(void) interrupt 0 {
    LOGB(0xbb);
}

void sendCommand(u8 CMD)
{
    u8 i;
    LOGB(CMD);
    ASK_SENDER_POWER_SWITCH = 1;
    //定时器0开启
    TR0 = 1; 
    //发送数据
    delay_ms(1);
    
    //自己的设备ID
    for(i = 0; i < 4; i++)
    {
        SimpleAskSender_SendBytes(&askSender,DEV_ID,7);
        SimpleAskSender_Send(&askSender,CMD);
        LOGB(i);
    }
    while (askSender.b_busy);
    LOGB(0xff);

    delay_ms(1);
    //暂停定时器0
    TR0 = 0; 
    
    ASK_SENDER_POWER_SWITCH = 0;
}





#define SLEEP_SHALLOW_TIMEOUT_COUNT  5
#define SLEEP_DEEPLY_COUNT  3

u8 mTargetStatus = STATUS_WORKING;
u8 shallowWakeupCount = 0;
u8 tiredForDeepSleepCounter=0;

void waitWhenBusy(){
    while (COM1.B_TX_busy);
    while (askSender.b_busy);
}


//小憩状态，由掉电唤醒定时器唤醒
void takeOneNap(void)
{
    LOGB(0X10);
    //开启定时器唤醒模式
    WKTCL = 0xFE;
    // WKTCH = 0xFF; //16s
    WKTCH = 0x87;// 1s
    while (shallowWakeupCount<SLEEP_SHALLOW_TIMEOUT_COUNT)
    {
        LOGB(0X11);
        waitWhenBusy();
        shallowWakeupCount++;
        MCU_POWER_DOWN();
    }
    LOGB(0X12);
    shallowWakeupCount=0;
    tiredForDeepSleepCounter++;
    //关闭定时器唤醒模式
    WKTCH = 0x7F;
}

void sleepDeeply(void)
{
    LED=0;
    sendCommand(CMD_SLEEPING);
    waitWhenBusy();
    //打开外部中断0
    EX0 =1;
    MCU_POWER_DOWN();
    //关闭外部中断0
    EX0 = 0;
    mTargetStatus=STATUS_WORKING;
}

void working(void)
{
    LED=1;
    sendCommand(CMD_WORKING);
    takeOneNap();
    if(tiredForDeepSleepCounter>SLEEP_DEEPLY_COUNT){
        tiredForDeepSleepCounter=0;
        mTargetStatus=STATUS_SLEEPING;
    }else{
        mTargetStatus=STATUS_WORKING;
    }
}

void main(void){
    Child_Init();
    sendCommand(CMD_MATCHING);
    waitWhenBusy();
    while (1)
    {
        if(mTargetStatus==STATUS_SLEEPING){
            sleepDeeply();
        }else{
            working();
        }
        // delay_ms(255);
        // delay_ms(255);
        // delay_ms(255);
        // delay_ms(255);
        // LED=~LED;
    }
}