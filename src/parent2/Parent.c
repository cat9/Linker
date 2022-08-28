#include "Parent.h"

#include "Device.h"

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

void Deviceinit()
{
    P1_MODE_IO_PU(GPIO_Pin_6); // P1.6 LED灯引脚

    P3_MODE_IO_PU(GPIO_Pin_2); // P3.2 KEY0引脚
    P3_MODE_IO_PU(GPIO_Pin_3); // P3.3 KEY1引脚
    P3_MODE_IO_PU(GPIO_Pin_6); // P3.6 KEY2引脚
    P3_MODE_IO_PU(GPIO_Pin_7); // P3.7 KEY3引脚
    //拉高电平才能接收到输入信号
    KEY0_PIN = 1;
    KEY1_PIN = 1;
    KEY2_PIN = 1;
    KEY3_PIN = 1;

    IT0 = 1; //中断0 下降沿中断  P3.2 按键引脚
    EX0 = 1; //开启中断  P3.2 按键引脚
    IT1 = 1; //中断0 下降沿中断  P3.2 按键引脚
    EX1 = 1; //开启中断  P3.2 按键引脚
    INTCLKO = 0x30; //使能INT2 及 INT3 中断 (只支持下降沿中断)

    P1_MODE_OUT_PP(GPIO_Pin_0); // P1.0 蜂鸣器引脚
    BEEP_PIN = 0;

    P1_MODE_IN_HIZ(GPIO_Pin_1); // P1.1 设置为高阻输入 ASK接收引脚

    UART_config();
    SimpleAskReceiver_Init(&askReceiver, 8, 255, 400, 1200);
    CCON = 0x00;
    CMOD = 0x00;
    CL = 0x00;
    CH = 0x00;
    ASK_CCAPL = 0x00;
    ASK_CCAPH = 0x00;
    ASK_CCAPM = 0x31; //上升沿及下降沿触发捕获

    Device_Init();
    Device_ResetAllUserData(); // 清除所有用户除了设备ID外的配置数据，不需要
    Device_PrintData();        // 打印设备ID和配置数据

    Timer0Init();

    BEEP_PIN = 0;

    TR0 = 1; //定时器0开始计时
    EA = 1;
    CR = 1;
    busyTimeCouter=0;

    //低电压检测
    PCON &= ~LVDF;
    RSTCFG = LVD3V0;  //低压配置
    ELVD = 1;  //开启低电压中断
}



#define waitWhenBusy() while (COM1.B_TX_busy)

u8 buffer_receive[ASK_FRAME_SIZE];
u8 read;
u8 i;


u16 powerDownCount = 0;
s8 deviceIndex = -1;
u8 cmd;

#define DEVICE_TIMEOUT 60

void setDeviceStatus(s8 devIndex, u8 status)
{
    u8 *userDataPtr = Device_GetUserDataPtr(devIndex);
    *userDataPtr = status;
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
            // DEBUG_MAIN(totalTime);
            if (totalTime > DEVICE_TIMEOUT)
            {
                *userDataPtr = STATUS_MISSING;
                *(userDataPtr + 1) = 0x00;
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

void setAllMissingDeviceToIgnore(){
    u8 devCount, i;
    devCount = Device_GetDeviceCount();
    for (i = 0; i < devCount; i++)
    {
        u8 *userDataPtr = Device_GetUserDataPtr(i);
        if (*userDataPtr == STATUS_MISSING)
        {
            *userDataPtr = STATUS_MISSING_IGNORE;
            *(userDataPtr + 1) = 0x00;
        }
    }
}

u8 hasMissingDevice(){
    u8 devCount, i;
    devCount = Device_GetDeviceCount();
    for (i = 0; i < devCount; i++)
    {
        u8 *userDataPtr = Device_GetUserDataPtr(i);
        if (*userDataPtr == STATUS_MISSING)
        {
            return 1;
        }
    }
    return 0;
}



void main()
{
    Deviceinit();
    LOGB(0xaa);
    waitWhenBusy();
    // SimpleLed_Start(&led, 5, 5, 0);
    //  SimpleBeep_Start(&beep, 20, 250, 0);
    while (1)
    {
        read = SimpleAskReceiver_Read(&askReceiver, buffer_receive, ASK_FRAME_SIZE);
        if (read == ASK_FRAME_SIZE)
        {
            for (i = 0; i < read; i++)
            {
                LOGB(buffer_receive[i]);
            }
            deviceIndex = Device_IndexOf(buffer_receive);
            cmd = buffer_receive[CMD_INDEX];

            if (deviceIndex == -1)
            {
                LOGB(0xA0);
                if (cmd == CMD_MATCHING && SimpleKey_IsLongPressed(&key0))
                {
                    SimpleKey_Consumed(&key0);
                    deviceIndex = Device_Add(buffer_receive);
                    if (deviceIndex < 0)
                    {
                        DEBUG_MAIN(0xEE);
                    }
                    else
                    {
                        Device_Save();
                        // //设备运行状态不用保存
                        setDeviceStatus(deviceIndex,STATUS_WORKING);
                        DEBUG_MAIN(0xEA);
                    }
                }
                DEBUG_MAIN(0xEB);
            }
            else
            {
                if (cmd == CMD_WORKING)
                {
                    DEBUG_MAIN(0xEC);
                    setDeviceStatus(deviceIndex,STATUS_WORKING);
                }
                else if (cmd == CMD_SLEEPING)
                {
                    DEBUG_MAIN(0xED);
                    setDeviceStatus(deviceIndex,STATUS_SLEEPING);
                }
                else if (cmd == CMD_POWER_LESS)
                {
                    DEBUG_MAIN(0xED);
                    if(!beep.busy){
                        SimpleBeep_Start(&beep, BEEP_POWER_LESS_OTHER, 5, 255, 0);
                    }
                    
                }
                else
                {
                    DEBUG_MAIN(cmd);
                }

                //如果没有missing的设备，关闭蜂鸣器
                if(beep.busy && beep.beep_config==BEEP_MISSING && (cmd == CMD_WORKING || cmd == CMD_SLEEPING) && !hasMissingDevice()){
                    SimpleBeep_Stop(&beep);
                }
            }
        }
        else
        {
            if(beep.busy && SimpleKey_IsClicked(&key0))
            {
                SimpleKey_Consumed(&key0);
                SimpleBeep_Stop(&beep);
                setAllMissingDeviceToIgnore();
            }
            if (key0.busy || key1.busy || key2.busy || key3.busy || beep.busy)
            {
                if(busyTimeCouter>1000){
                    read = busyTimeCouter / 1000;
                    DEBUG_MAIN(read);
                    if (checkDeviceStatus(read))
                    {
                        DEBUG_MAIN(0xE8);
                        if(!beep.busy){
                            SimpleBeep_Start(&beep, BEEP_MISSING, 5, 255, 0);
                        }
                    }
                    busyTimeCouter=0;
                    DEBUG_MAIN(0xFd);
                }
                continue;
            }

            // LOGB(0xaa);
            // LOGB(AskReceiver.b_busy);
            if (askReceiver.busy_counter == 0)
            {

                DEBUG_MAIN(0x55);
                waitWhenBusy();
                if (key0.busy || key1.busy || key2.busy || key3.busy || beep.busy){
                    continue;
                }
                ET0 = 0;
                TR0 = 0;
                busyTimeCouter = 0;

                // ASK关闭
                CR = 0;
                ASK_CCAPL = 0x00;
                ASK_CCAPH = 0x00;
                CL = 0x00;
                CH = 0x00;
                ASK_CCF = 0;
                CF = 0;
                askReceiver.last_value = 0;

                WKTCL = 0x30;
                WKTCH = 0xF5; // 15s
                if (key0.busy || key1.busy || key2.busy || key3.busy || beep.busy){
                    DEBUG_MAIN(0x56);
                }else{
                    MCU_POWER_DOWN();
                }
                
                CR = 1;
                ET0 = 1;
                TR0 = 1;

                WKTCH = WKTCH & 0x7F;
                DEBUG_MAIN(WKTCH);
                DEBUG_MAIN(WKTCL);
                ((u8 *)&powerDownCount)[0] = WKTCH & 0x7F;
                ((u8 *)&powerDownCount)[1] = WKTCL;
                if (powerDownCount == 0 && askReceiver.busy_counter == 0)
                {
                    powerDownCount = 0x7530;
                }
                else
                {
                    powerDownCount = powerDownCount + 1;
                }
                read = powerDownCount / 2000;
                powerDownCount = powerDownCount % 2000;
                if (powerDownCount > 1000)
                {
                    read++;
                }
                DEBUG_MAIN(read);
                if (read > 0 && checkDeviceStatus(read))
                {
                    DEBUG_MAIN(0xE9);
                    SimpleBeep_Start(&beep, BEEP_MISSING, 5, 255, 0);
                }
                DEBUG_MAIN(0x66);
            }
        }
    }
}