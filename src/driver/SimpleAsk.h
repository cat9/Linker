#ifndef __SIMPLE_ASK_H__

#include "../config.h"

//开关
// #define ASK_SENDER 1
// #define ASK_RECEIVER 1

/*---------------------------------------------------------------------*/
/*   目前实现的是固定长度的异步收发                                      */
/*---------------------------------------------------------------------*/

#define ASK_FRAME_SIZE 8 //每帧数据的字节长度，每次发送/接收一帧数据
#define ASK_RECEIVER_BUFFER_LENGTH 64 // 发送器缓存区字节数量

/*======================================================================================*/
/*======================================================================================*/
/*======================================================================================*/

#define SIMPLE_ASK_RECEIVER

#ifdef SIMPLE_ASK_RECEIVER

typedef struct SimpleAskReceiver
{
    u16 (*read_capture_counter_value)();
    bit (*read_capture_ping_value)();

    u8 repeat_counter;//去重计数器
    u8 repeat_reload; //去重计数器重新装载值
    

    u8 busy_counter; //接收器忙计数器
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

    u8 receive_ping_value:1;     //引脚电平值
    u8 receive_hight_flag:1;     //高电平长短标志 0:短 1:长
    u8 receive_low_flag:1;       //低电平长短标志 0:短 1:长
    u8 receive_hight_flag_set:1; //高电平是否接收完成标志 0:未完成 1:完成
    u8 receive_low_flag_set:1;   //低电平是否接收完成标志 0:未完成 1:完成

    u8 receiver_buffer[ASK_RECEIVER_BUFFER_LENGTH]; //接收缓冲区
    u8 frame_buffer[ASK_FRAME_SIZE];       //接收帧缓冲区
    u8 frame_buffer_last[ASK_FRAME_SIZE];       //去重帧缓冲区
} xdata SimpleAskReceiver;

void SimpleAskReceiver_Init(SimpleAskReceiver *simpleAskReceiver,u8 busy_reload,u8 repeat_reload,u16 short_time_us,u16 long_time_us);
u8 SimpleAskReceiver_Read(SimpleAskReceiver *simpleAskReceiver,u8 *buff, u8 len);
void SimpleAskReceiver_HeartBeat(SimpleAskReceiver *simpleAskReceiver);


#endif // SIMPLE_ASK_RECEIVER


/*======================================================================================*/


#define SIMPLE_ASK_SENDER
#ifdef SIMPLE_ASK_SENDER // ASK发送器

#define ASK_SENDER_BUFFER_LENGTH 64                      // 发送器缓存区字节数量

//协议波形配置(定时器为50us)
// #define ASK_SENDER_DELAY_LONG 24 // 1.2ms
// #define ASK_SENDER_DELAY_SHORT 8 // 400us
// #define ASK_SENDER_DELAY_SYNC 96 // 4.8ms

typedef struct SimpleAskSender
{
    void (*write_pin_value)(const u8 value);
    u8 read;           //发送读指针
    u8 write;          //发送写指针
    u8 byte_now;       //当前要发送的字节
    u8 byte_bit_sent;  //当前字节已发送bit数
    u8 frame_bit_sent; //当前帧已发送bit数
    u8 bit_switcher;   // bit电位切换计数标志 0-16,偶数高电平，奇数低电平

    u8 delay_short; //短延时
    u8 delay_long;  //长延时
    u8 delay_sync;  //同步延时

    u8 b_busy;      //是否正在发送
    u8 time_couter; //时间计数器 每次50us
    u8 sender_buffer[ASK_SENDER_BUFFER_LENGTH]; //发送缓冲
} xdata SimpleAskSender;

void SimpleAskSender_Init(SimpleAskSender *simpleAskSender,u16 heart_beat_us,u16 short_time_us,u16 long_time_us);

u8 SimpleAskSender_IsBusy(SimpleAskSender *simpleAskSender);

void SimpleAskSender_Send(SimpleAskSender *simpleAskSender,u8 dat);

void SimpleAskSender_SendBytes(SimpleAskSender *simpleAskSender,u8 buff[], u8 len);

void SimpleAskSender_HeartBeat(SimpleAskSender *simpleAskSender);

#endif // ASK_SENDER


#endif // !__SIMPLE_ASK_H__