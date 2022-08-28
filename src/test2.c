#include "driver/UART.h"
#include "driver/Device.h"
#include "driver/Key.h"
#include "driver/ASK.h"

#define CMD_INDEX 7

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

void Timer0_ISR_Handler(void) interrupt 1
{
    // LOGB(0x99);
    if (AskReceiver.b_busy > 0)
    {
        AskReceiver.b_busy--;
    }

    // 1ms
    Key_Detect();
}

void init()
{
    P3_MODE_IN_HIZ(GPIO_Pin_3); // P3.3 设置为高阻输入 ASK接收引脚
    P3_MODE_IO_PU(GPIO_Pin_2);  // P3.2 按键引脚
    P5_MODE_IO_PU(GPIO_Pin_5);  // P5.5 LED灯

    // P32 = 1;

    // P3_MODE_IO_PU(GPIO_Pin_3);
    Key_Init();
    UART_config();

    Device_Init();
    Device_ResetAllUserData(); // 清除所有用户除了设备ID外的配置数据，不需要
    Device_PrintData();        // 打印设备ID和配置数据

    Ask_ReceiverInit(8);
    Timer0Init();

    IT0 = 1; //中断0 下降沿中断  P3.2 按键引脚
    EX0 = 1; //开启中断  P3.2 按键引脚

    EA = 1;
}

#define waitWhenBusy() while (COM1.B_TX_busy)

u8 buffer_receive[ASK_FRAME_SIZE];
u8 read;
u8 i;

#define LED P55

void INT1_INTERUPT() interrupt 2
{
    LOGB(0x88);
    EX1 = 0;
}

void INT0_INTERUPT() interrupt 0
{
    LOGB(0x66);
}

u16 powerDownCount = 0;
s8 deviceIndex = -1;
u8 cmd;

#define DEVICE_TIMEOUT 60

void setDeviceWorking(s8 devIndex)
{
    u8 *userDataPtr = Device_GetUserDataPtr(devIndex);
    *userDataPtr = STATUS_WORKING;
    *(userDataPtr + 1) = 0x00;
}

void setDeviceSleeping(s8 devIndex)
{
    u8 *userDataPtr = Device_GetUserDataPtr(devIndex);
    *userDataPtr = STATUS_SLEEPING;
    *(userDataPtr + 1) = 0x00;
}

void setDeviceMissing(s8 devIndex)
{
    u8 *userDataPtr = Device_GetUserDataPtr(devIndex);
    *userDataPtr = STATUS_SLEEPING;
    *(userDataPtr + 1) = 0x00;
}

u8 checkDeviceStatus(u8 diffTime)
{
    u8 devCount, i, totalTime;
    u8 hasNewMissing = 0;
    devCount = Device_GetDeviceCount();
    for (i = 0; i < devCount; i++)
    {
        u8 *userDataPtr = Device_GetUserDataPtr(i);
        if (*userDataPtr == STATUS_WORKING)
        {
            totalTime = *(userDataPtr + 1) + diffTime;
            LOGB(totalTime);
            if (totalTime > DEVICE_TIMEOUT)
            {
                *userDataPtr = STATUS_MISSING;
                *(userDataPtr + 1) = 0;
                hasNewMissing = 1;
            }
            else
            {
                *(userDataPtr + 1) += diffTime;
            }
        }
    }
    return hasNewMissing;
}

void main()
{
    init();
    LOGB(0xaa);
    waitWhenBusy();
    while (1)
    {
        // read = Ask_ReceiverRead(buffer_receive, ASK_FRAME_SIZE);
        // if (read == ASK_FRAME_SIZE)
        // {
        //     for (i = 0; i < read; i++)
        //     {
        //         LOGB(buffer_receive[i]);
        //     }
        //     deviceIndex = Device_IndexOf(buffer_receive);
        //     cmd = buffer_receive[CMD_INDEX];

        //     if (deviceIndex == -1)
        //     {
        //         if (cmd == CMD_MATCHING && Key_IsLongPressed())
        //         {
        //             Key_Consumed();
        //             deviceIndex = Device_Add(buffer_receive);
        //             if (deviceIndex < 0)
        //             {
        //                 LOGB(0xEE);
        //             }
        //             else
        //             {
        //                 Device_Save();
        //                 //设备运行状态不用保存
        //                 setDeviceWorking(deviceIndex);
        //                 LOGB(0xEA);
        //             }
        //         }
        //         LOGB(0xEB);
        //     }
        //     else
        //     {
        //         if (cmd == CMD_WORKING)
        //         {
        //             LOGB(0xEC);
        //             setDeviceWorking(deviceIndex);
        //         }
        //         else if (cmd == CMD_SLEEPING)
        //         {
        //             LOGB(0xED);
        //             setDeviceSleeping(deviceIndex);
        //         }
        //         else
        //         {
        //             LOGB(cmd);
        //         }
        //     }
        // }
        // else
        // {
        //     if (!Key_IsBusy())
        //     {
        //         // LOGB(0xaa);
        //         // LOGB(AskReceiver.b_busy);
        //         if (AskReceiver.b_busy == 0)
        //         {

        //             LOGB(0x55);
        //             waitWhenBusy();
        //             LED = 0;
        //             ET0 = 0;
        //             TR0 = 0;

        //             // ASK关闭
        //             CR = 0;
        //             ASK_CCAPL = 0x00;
        //             ASK_CCAPH = 0x00;
        //             CL = 0x00;
        //             CH = 0x00;
        //             CCF1 = 0;
        //             CF = 0;
        //             AskReceiver.last_value = 0;
        //             // IT1 = 0; //INT1上升沿下降沿触发
        //             // EX1 = 1; //使能INT1中断

        //             // WKTCL = 0xFE;
        //             // WKTCH = 0xFF; // 16s

        //             WKTCL = 0x30;
        //             WKTCH = 0xF5; // 15s
        //             MCU_POWER_DOWN();
        //             CR = 1;
        //             ET0 = 1;
        //             TR0 = 1;

        //             // WKTCH = WKTCH & 0x7F;
        //             LOGB(WKTCH);
        //             LOGB(WKTCL);
        //             ((u8 *)&powerDownCount)[0] = WKTCH & 0x7F;
        //             ((u8 *)&powerDownCount)[1] = WKTCL;
        //             if (powerDownCount == 0 && AskReceiver.b_busy==0)
        //             {
        //                 powerDownCount = 0x7530;
        //             }
        //             else
        //             {
        //                 powerDownCount = powerDownCount + 1;
        //             }
        //             read = powerDownCount / 2000;
        //             powerDownCount = powerDownCount % 2000;
        //             if(powerDownCount>1000){
        //                 read++;
        //             }
        //             LOGB(read);
        //             if (i > 0 && checkDeviceStatus(read))
        //             {
        //                 LOGB(0xE9);
        //             }

        //             LED = 1;
        //             LOGB(0x66);
        //         }
        //     }
        // }

        // LOGB(0x77);
        // WKTCL = 0xFE;
        // WKTCH = 0xFF; // 16s
        // LED = 0;
        // ET0 = 0;
        // TR0 = 0;
        // waitWhenBusy();
        // MCU_POWER_DOWN();
        // WKTCH = WKTCH & 0x7F;
        // LED = 1;
        // ET0 = 1;
        // TR0 = 1;
        // LOGB(WKTCH);
        // LOGB(WKTCL);
        // ((u8 *)&powerDownCount)[0] = WKTCH;
        // ((u8 *)&powerDownCount)[1] = WKTCL;
        // if (powerDownCount == 0)
        // {
        //     powerDownCount = 0x7fff;
        // }
        // else
        // {
        //     powerDownCount = powerDownCount + 1;
        // }
        // i = powerDownCount / 2000;
        // LOGB(i);
        // if (i > 0 && checkDeviceStatus(i))
        // {
        //     LOGB(0xE9);
        // }
        // LOGB(0x7F);

        // read = Ask_ReceiverRead(buffer_receive, ASK_FRAME_SIZE);
        // if (read == ASK_FRAME_SIZE)
        // {
        //     for (i = 0; i < ASK_FRAME_SIZE; i++)
        //     {
        //         LOGB(buffer_receive[i]);
        //     }
        // }
        // // LOGB(AskReceiver.b_busy);
        // if(AskReceiver.b_busy==0){
        //     LOGB(0x55);
        //     waitWhenBusy();
        //     LED=0;
        //     ET0 = 0;
        //     TR0 = 0;
        //     CR = 0;
        //     ASK_CCAPL = 0x00;
        //     ASK_CCAPH = 0x00;
        //     CL = 0x00;
        //     CH = 0x00;
        //     CCF1 = 0;
        //     CF = 0;
        //     AskReceiver.last_value=0;
        //     IT1=0;
        //     EX1=1;
        //     MCU_POWER_DOWN();
        //     CR = 1;
        //     ET0 = 1;
        //     TR0 = 1;
        //     LED=1;
        //     LOGB(0x66);
        // }
        // LOGB(0x77);
        // WKTCL = 0xFE;
        // WKTCH = 0xFF; //16s
        // // WKTCH = 0x87;// 1s
        // LED=0;
        // ET0 = 0;
        // TR0 = 0;
        // waitWhenBusy();
        // MCU_POWER_DOWN();
        // LED=1;
        // ET0 = 1;
        // TR0 = 1;
        // delay_ms(255);
        // delay_ms(255);
        // delay_ms(255);
        // delay_ms(255);

        // if(Key_IsLongPressed()){
        //     Key_Consumed();
        //     LOGB(0xbb);
        //     LED = !LED;
        //     delay_ms(100);
        // }
        // if(Key_IsClicked()){
        //     Key_Consumed();
        //     LOGB(0xcc);
        //     LED = !LED;
        //     delay_ms(100);
        // }
    }
}