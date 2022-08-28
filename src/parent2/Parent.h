#ifndef __PARENT_H__
#define __PARENT_H__

#include "../config.h"
#include "../driver/SimpleBeep.h"
#include "../driver/SimpleKey.h"
#include "../driver/SimpleAsk.h"
#include "../driver/SimpleLed.h"
#include "../driver/UART.h"

#define DEBUG_MAIN(BYTE) TX1_write2buff(BYTE)
// #define DEBUG_MAIN(BYTE)



/** ==================[按键 start]================== **/
#define KEY0_PIN P32
#define KEY1_PIN P33
#define KEY2_PIN P36
#define KEY3_PIN P37

bit read_key0_value()
{
    return KEY0_PIN;
}
const SimpleKey key0 = {read_key0_value, 0, 0, 0, 0, 0};
void key0_Interrupt() interrupt 0
{
    key0.busy=1;
}

bit read_key1_value()
{
    return KEY1_PIN;
}
const SimpleKey key1 = {read_key1_value, 0, 0, 0, 0, 0};
void key1_Interrupt() interrupt 2
{
    key1.busy=1;
}

bit read_key2_value()
{
    return KEY2_PIN;
}
const SimpleKey key2 = {read_key2_value, 0, 0, 0, 0, 0};
void key2_Interrupt() interrupt 10
{
    LOGB(0x36);
    key2.busy=1;
}

bit read_key3_value()
{
    return KEY3_PIN;
}
const SimpleKey key3 = {read_key3_value, 0, 0, 0, 0, 0};
void key3_Interrupt() interrupt 11
{
    LOGB(0x37);
    key3.busy=1;
}

/** ==================[按键 end]================== **/

/** ==================[LED start]================== **/

#define LED_PIN P16

void write_led_value(const u8 value)
{
    LED_PIN = value;
}

const SimpleLed led = {write_led_value, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

/** ==================[LED end]================== **/


/** ==================[蜂鸣器 start]================== **/

#define BEEP_PIN P10

void write_beep_value(const u8 value)
{
    BEEP_PIN = value;
}

const SimpleBeep beep = {write_beep_value, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

/** ==================[蜂鸣器 end]================== **/



/** ==================[Ask接收器 start]================== **/

#define ASK_RECEIVER_PIN P11 //接收端口
#define ASK_CCAPL CCAP0L     //接收端口对应的CCAPnL寄存器 低位
#define ASK_CCAPH CCAP0H     //接收端口对应的CCAPnH寄存器 高位
#define ASK_CCAPM CCAPM0     //接收端口对应的CCAPMn寄存器 模式位
#define ASK_CCF CCF0         //接收端口对应的PAC中断请求标志位
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
    // DEBUG_MAIN(0xCC);

    if (ASK_CCF)
    {
        ASK_CCF = 0;
        CF = 0;
        // DEBUG_MAIN(0xCB);
        // DEBUG_MAIN(ASK_RECEIVER_PIN);

        SimpleAskReceiver_HeartBeat(&askReceiver);
    }
}

/** ==================[Ask接收器 end]================== **/



u16 busyTimeCouter=0;
void Timer0_ISR_Handler(void) interrupt 1
{
    // DEBUG_MAIN(0x99);
    if (askReceiver.busy_counter > 0)
    {
        askReceiver.busy_counter--;
    }
    if (askReceiver.repeat_counter > 0)
    {
        askReceiver.repeat_counter--;
    }

    // 1ms
    busyTimeCouter++;

    SimpleKey_HeartBeat(&key0);
    SimpleKey_HeartBeat(&key1);
    SimpleKey_HeartBeat(&key2);
    SimpleKey_HeartBeat(&key3);

    SimpleBeep_HeartBeat(&beep);
    SimpleLed_HeartBeat(&led);
}



#endif // !__PARENT_H__