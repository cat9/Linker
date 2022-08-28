#ifndef __ASK_H__
#define __ASK_H__

#include "config.h"

//开关
// #define ASK_SENDER 1
// #define ASK_RECEIVER 1

/*---------------------------------------------------------------------*/
/*   目前实现的是固定长度的异步收发                                      */
/*---------------------------------------------------------------------*/

#define ASK_FRAME_SIZE 8 //每帧数据的字节长度，每次发送/接收一帧数据

/*======================================================================================*/
/*======================================================================================*/
/*======================================================================================*/

#ifdef ASK_RECEIVER

//使用PCA实现的接收功能

// #ifdef STC8G1K08A
    #define ASK_RECEIVER_PIN P33 //接收端口
    #define ASK_CCAPL CCAP1L //接收端口对应的CCAPnL寄存器 低位
    #define ASK_CCAPH CCAP1H //接收端口对应的CCAPnH寄存器 高位
    #define ASK_CCAPM CCAPM1 //接收端口对应的CCAPMn寄存器 模式位
    #define ASK_CCF CCF1 //接收端口对应的PAC中断请求标志位
// #endif // STC8G1K08A

// #ifdef STC8G1K08_38I_QFN20
//     #define ASK_RECEIVER_PIN P11 //接收端口
//     #define ASK_CCAPL CCAP0L //接收端口对应的CCAPnL寄存器 低位
//     #define ASK_CCAPH CCAP0H //接收端口对应的CCAPnH寄存器 高位
//     #define ASK_CCAPM CCAPM0 //接收端口对应的CCAPMn寄存器 模式位
//     #define ASK_CCF CCF0 //接收端口对应的PAC中断请求标志位
// #endif // STC8G1K08_38I_QFN20


#define ASK_RECEIVER_BUFFER_LENGTH 64 // 发送器缓存区字节数量

typedef struct
{
    u8 b_busy; //接收器是否忙
    u8 busy_reload; //接收器忙重新装载值
    u8 receive_bit_count; //当前收到的bit数量
    u8 receive_byte;      //当前收到的byte

    u8 receive_buff_write_index; //缓冲区写索引
    u8 receive_buff_read_index;  //缓冲区读索引

    u8 receive_buff_frame_index; //接收帧缓冲区索引

    u16 signal_length; //电平长度
    u16 now_value;     //当前中断值
    u16 last_value;    //上次中断值

    u16 bound_short_delay_lower; //短信号时长下限
    u16 bound_short_delay_upper; //短信号时长上限

    u16 bound_long_delay_lower; //长信号时长下限
    u16 bound_long_delay_upper; //长信号时长上限
} Ask_Receiver_Define;

extern Ask_Receiver_Define AskReceiver;

void Ask_ReceiverInit(u8 busy_reload);

u8 Ask_ReceiverRead(u8 *dat, u8 len);

#endif // ASK_RECEIVER

/*======================================================================================*/
/*======================================================================================*/
/*======================================================================================*/

#ifdef ASK_SENDER // ASK发送器

#ifdef STC8G1K08A
#define ASK_SENDER_PIN P33 //发送端口
#endif

#ifdef STC8G1K17
#define ASK_SENDER_PIN P36 //发送端口
#endif                     // STC8G1K17

#define ASK_SENDER_BUFFER_LENGTH 64                      // 发送器缓存区字节数量
#define ASK_SENDER_FRAME_BIT_LENGTH (ASK_FRAME_SIZE * 8) // 发送器每帧数据的位长度=ASK_FRAME_SIZE*8

//协议波形配置(定时器为50us)
#define ASK_SENDER_DELAY_LONG 24 // 1.2ms
#define ASK_SENDER_DELAY_SHORT 8 // 400us
#define ASK_SENDER_DELAY_SYNC 96 // 4.8ms

typedef struct
{
    u8 read;           //发送读指针
    u8 write;          //发送写指针
    u8 byte_now;       //当前要发送的字节
    u8 byte_bit_sent;  //当前字节已发送bit数
    u8 frame_bit_sent; //当前帧已发送bit数
    u8 bit_switcher;   // bit电位切换计数标志 0-16,偶数高电平，奇数低电平

    u8 b_busy;      //是否正在发送
    u8 time_couter; //时间计数器 每次50us
} Ask_Sender_Define;

extern Ask_Sender_Define AskSender;

void Ask_SenderInit(void);

u8 Ask_IsBusy(void);

void Ask_Send(u8 dat);

void Ask_SendBytes(u8 buff[], u8 len);

void Ask_SenderProcess();

#endif // ASK_SENDER

#endif // __ASK_H__
