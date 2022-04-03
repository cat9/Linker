#include "ASK.h"
#include "UART.h"

#include "stdio.h"

#ifdef ASK_RECEIVER

// #define DEBUG_RECEIVER(BYTE) TX1_write2buff(BYTE)
#define DEBUG_RECEIVER(BYTE)

bit receive_ping_value;

bit receive_hight_flag;     //高电平长短标志 0:短 1:长
bit receive_low_flag;       //低电平长短标志 0:短 1:长
bit receive_hight_flag_set; //高电平是否接收完成标志 0:未完成 1:完成
bit receive_low_flag_set;   //低电平是否接收完成标志 0:未完成 1:完成

u8 receive_bit_count; //当前收到的bit数量
u8 receive_byte;      //当前收到的byte

u8 receive_buff_write_index;                              //缓冲区写索引
u8 receive_buff_read_index;                               //缓冲区读索引
u8 xdata ask_receiver_buffer[ASK_RECEIVER_BUFFER_LENGTH]; //接收缓冲区

u8 receive_buff_frame_index;                        //接收帧缓冲区索引
u8 xdata ask_receiver_frame_buffer[ASK_FRAME_SIZE]; //接收帧缓冲区

u16 signal_length; //电平长度
u16 now_value;     //当前中断值
u16 last_value;    //上次中断值

u16 bound_short_delay_lower; //短信号时长下限
u16 bound_short_delay_upper; //短信号时长上限

u16 bound_long_delay_lower; //长信号时长下限
u16 bound_long_delay_upper; //长信号时长上限

void Ask_ReceiverInit(void)
{

    // P3_MODE_IN_HIZ(GPIO_Pin_3); // P3.3 设置为高阻输入

    receive_ping_value = 0;

    receive_hight_flag = 0;     //高电平长短标志 0:短 1:长
    receive_low_flag = 0;       //低电平长短标志 0:短 1:长
    receive_hight_flag_set = 0; //高电平是否接收完成标志 0:未完成 1:完成
    receive_low_flag_set = 0;   //低电平是否接收完成标志 0:未完成 1:完成

    receive_bit_count = 0; //当前收到的bit数量
    receive_byte = 0;      //当前收到的byte

    receive_buff_write_index = 0; //缓冲区写索引
    receive_buff_read_index = 0;  //缓冲区读索引
    receive_buff_frame_index = 0; //接收帧缓冲区索引

    signal_length = 0; //电平长度
    now_value = 0;     //当前中断值
    last_value = 0;    //上次中断值

    //信号70%
    bound_short_delay_lower = 400 - 400 * 3 / 10;
    bound_short_delay_upper = 400 + 400 * 3 / 10;
    bound_long_delay_lower = 1200 - 1200 * 3 / 10;
    bound_long_delay_upper = 1200 + 1200 * 3 / 10;

    CCON = 0x00;
    CMOD = 0x00;
    CL = 0x00;
    CH = 0x00;
    ASK_CCAPL = 0x00;
    ASK_CCAPH = 0x00;
    ASK_CCAPM = 0x31; //上升沿及下降沿触发捕获
    CR = 1;        //启动
    EA = 1;
}

u8 Ask_ReceiverRead(u8 *dat, u8 len)
{
    u8 i;
    // LOGB(0xfa);
    // LOGB(0xc1);
    // LOGB(receive_buff_read_index);
    // LOGB(receive_buff_write_index);
    for (i = 0; i < len && receive_buff_read_index != receive_buff_write_index; i++)
    {
        dat[i] = ask_receiver_buffer[receive_buff_read_index++];
        DEBUG_RECEIVER(dat[i]);
        if (receive_buff_read_index >= ASK_RECEIVER_BUFFER_LENGTH)
        {
            receive_buff_read_index = 0;
        }
    }
    // LOGB(i);
    // LOGB(0xfe);
    return i;
}

void Ask_ReceiverProcess()
{
    receive_hight_flag_set = 0;
    receive_low_flag_set = 0;
    DEBUG_RECEIVER(0xfd);
    if (receive_hight_flag)
    {
        DEBUG_RECEIVER(0x99);
    }
    else
    {
        DEBUG_RECEIVER(0xaa);
    }
    if (receive_low_flag)
    {
        DEBUG_RECEIVER(0x99);
    }
    else
    {
        DEBUG_RECEIVER(0xaa);
    }

    if (receive_hight_flag ^ receive_low_flag)
    {
        receive_byte <<= 1;
        if (receive_hight_flag)
        {
            //高长低短 bit=1
            receive_byte |= 0x01;
        }
        else
        {
            //高短低长 bit=0
            receive_byte &= 0xfe;
        }
        receive_bit_count++;
        DEBUG_RECEIVER(receive_bit_count);
    }
    else
    {
        DEBUG_RECEIVER(0xf4);
        receive_hight_flag_set = 0;
        receive_low_flag_set = 0;

        receive_byte = 0;
        receive_bit_count = 0;
        receive_buff_frame_index = 0;
        return;
    }

    if (receive_bit_count > 7)
    {
        //接收完一个byte
        DEBUG_RECEIVER(0xf5);
        receive_bit_count = 0;
        ask_receiver_frame_buffer[receive_buff_frame_index] = receive_byte;
        receive_buff_frame_index++;
        DEBUG_RECEIVER(receive_byte);
    }

    if (receive_buff_frame_index >= ASK_FRAME_SIZE)
    {
        DEBUG_RECEIVER(0xF6);
        //接收完成
        receive_hight_flag_set = 0;
        receive_low_flag_set = 0;
        receive_byte = 0;
        receive_bit_count = 0;
        for (receive_buff_frame_index = 0; receive_buff_frame_index < ASK_FRAME_SIZE; receive_buff_frame_index++)
        {
            DEBUG_RECEIVER(ask_receiver_frame_buffer[receive_buff_frame_index]);
            ask_receiver_buffer[receive_buff_write_index++] = ask_receiver_frame_buffer[receive_buff_frame_index];
            if (receive_buff_write_index >= ASK_RECEIVER_BUFFER_LENGTH)
            {
                receive_buff_write_index = 0;
            }
        }
        DEBUG_RECEIVER(receive_buff_read_index);
        DEBUG_RECEIVER(receive_buff_write_index);
        receive_buff_frame_index = 0;
    }
}

u8 hi = 0;
u8 low = 0;
void PCA_Interrupt() interrupt 7
{
    if (CCF1)
    {
        CCF1 = 0;
        CF = 0;

        DEBUG_RECEIVER(0xFA);
        ((u8 *)&now_value)[0] = CCAP1H;
        ((u8 *)&now_value)[1] = CCAP1L;

        if (now_value > last_value)
        {
            signal_length = now_value - last_value;
        }
        else
        {
            signal_length = 65536 - last_value + now_value;
        }

        last_value = now_value;

        receive_ping_value = ASK_RECEIVER_PIN;
        if (receive_ping_value)
        {
            DEBUG_RECEIVER(0x01);
        }
        else
        {
            DEBUG_RECEIVER(0x00);
        }

        hi = signal_length / 256;
        low = signal_length % 256;
        DEBUG_RECEIVER(hi);
        DEBUG_RECEIVER(low);

        // DEBUG_RECEIVER(0xfe);

        if (signal_length > bound_short_delay_lower && signal_length < bound_short_delay_upper)
        {
            //短信号
            DEBUG_RECEIVER(0xfb);
            if (receive_ping_value)
            {
                //上升沿,之前是低电平
                receive_low_flag = 0; //低短
                receive_low_flag_set = 1;
            }
            else
            {
                //下降沿，之前是高电平
                receive_hight_flag = 0; //高短
                receive_hight_flag_set = 1;
            }
            if (receive_hight_flag_set & receive_low_flag_set)
            {
                Ask_ReceiverProcess();
            }
            DEBUG_RECEIVER(0xfe);
        }
        else if (signal_length > bound_long_delay_lower && signal_length < bound_long_delay_upper)
        {
            //长信号
            DEBUG_RECEIVER(0xfc);
            if (receive_ping_value)
            {
                //上升沿,之前是低电平
                receive_low_flag = 1; //低长
                receive_low_flag_set = 1;
            }
            else
            {
                //下降沿，之前是高电平
                receive_hight_flag = 1; //高长
                receive_hight_flag_set = 1;
            }
            if (receive_hight_flag_set & receive_low_flag_set)
            {
                Ask_ReceiverProcess();
            }
            DEBUG_RECEIVER(0xfe);
        }
        else
        {
            //信号长度不对
            DEBUG_RECEIVER(0xf2);
            receive_hight_flag_set = 0;
            receive_low_flag_set = 0;

            receive_byte = 0;
            receive_bit_count = 0;
            receive_buff_frame_index = 0;
        }
    }
}

#endif // ASK_RECEIVER

#ifdef ASK_SENDER

// #define DEBUG_SENDER(BYTE) TX1_write2buff(BYTE)
#define DEBUG_SENDER(BYTE)

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

Ask_Sender_Define AskSender;
u8 xdata ask_sender_buffer[ASK_SENDER_BUFFER_LENGTH]; //发送缓冲

void Ask_SenderInit(void)
{
    AskSender.read = 0;
    AskSender.write = 0;
    AskSender.byte_now = 0;
    AskSender.byte_bit_sent = 0;
    AskSender.frame_bit_sent = 0;
    AskSender.bit_switcher = 0;
    AskSender.b_busy = 0;
    AskSender.time_couter = 0;
}

void Ask_Send(u8 dat)
{
    ask_sender_buffer[AskSender.write] = dat; //装发送缓冲
    if (++AskSender.write >= ASK_SENDER_BUFFER_LENGTH)
        AskSender.write = 0;
    if (AskSender.b_busy == 0)
    {
        AskSender.time_couter = 1;
        AskSender.b_busy = 1;
    }
}

void Ask_SendBytes(u8 buff[], u8 len)
{
    u8 i;
    for (i = 0; i < len; i++)
    {
        Ask_Send(buff[i]);
    }
}

void Ask_SenderProcess()
{
    if (AskSender.b_busy)
    {
        AskSender.time_couter--;
        if (AskSender.time_couter == 0)
        {
            DEBUG_SENDER(0xCC);
            if (AskSender.frame_bit_sent < ASK_SENDER_FRAME_BIT_LENGTH)
            {
                //取数据
                if (AskSender.byte_bit_sent == 0 && AskSender.bit_switcher == 0)
                {
                    if (AskSender.read != AskSender.write)
                    {
                        AskSender.byte_now = ask_sender_buffer[AskSender.read];
                    }
                    else
                    {
                        //缓冲区已空,等待下次数据
                        return;
                    }
                }

                //发送数据
                if (AskSender.byte_now & 0x80)
                {
                    DEBUG_SENDER(0xAA);
                    //发送1
                    if (AskSender.bit_switcher)
                    {
                        DEBUG_SENDER(0x00);
                        //后发低电平
                        ASK_SENDER_PIN = 0;
                        AskSender.bit_switcher = 0;
                        AskSender.time_couter = ASK_SENDER_DELAY_SHORT;
                        AskSender.frame_bit_sent++;
                        AskSender.byte_bit_sent++;
                        if (AskSender.byte_bit_sent > 7)
                        {
                            AskSender.byte_bit_sent = 0;
                            if (++AskSender.read >= ASK_SENDER_BUFFER_LENGTH)
                            {
                                AskSender.read = 0;
                            }
                        }
                        else
                        {
                            AskSender.byte_now = AskSender.byte_now << 1;
                        }
                    }
                    else
                    {
                        //先发的高电平
                        DEBUG_SENDER(0x01);
                        ASK_SENDER_PIN = 1;
                        AskSender.bit_switcher = 1;
                        AskSender.time_couter = ASK_SENDER_DELAY_LONG;
                    }
                }
                else
                {
                    DEBUG_SENDER(0xBB);
                    //发送0
                    if (AskSender.bit_switcher)
                    {
                        DEBUG_SENDER(0x00);
                        //后发低电平
                        ASK_SENDER_PIN = 0;
                        AskSender.bit_switcher = 0;
                        AskSender.time_couter = ASK_SENDER_DELAY_LONG;
                        AskSender.frame_bit_sent++;
                        AskSender.byte_bit_sent++;
                        if (AskSender.byte_bit_sent > 7)
                        {
                            AskSender.byte_bit_sent = 0;
                            if (++AskSender.read >= ASK_SENDER_BUFFER_LENGTH)
                            {
                                AskSender.read = 0;
                            }
                        }
                        else
                        {
                            AskSender.byte_now = AskSender.byte_now << 1;
                        }
                    }
                    else
                    {
                        DEBUG_SENDER(0x01);
                        //先发的高电平
                        ASK_SENDER_PIN = 1;
                        AskSender.bit_switcher = 1;
                        AskSender.time_couter = ASK_SENDER_DELAY_SHORT;
                    }
                }
            }
            else
            {
                DEBUG_SENDER(0xEE);
                //发送帧结束标志
                if (AskSender.bit_switcher)
                {
                    //后发低电平
                    DEBUG_SENDER(0x00);

                    if (AskSender.frame_bit_sent > ASK_SENDER_FRAME_BIT_LENGTH)
                    {
                        DEBUG_SENDER(0xFB);
                        //这里等待最后的同步帧低电平完成再结束
                        AskSender.bit_switcher = 0;
                        AskSender.frame_bit_sent = 0;
                        if (AskSender.read == AskSender.write)
                        {
                            DEBUG_SENDER(0xFE);
                            AskSender.b_busy = 0;
                        }
                    }
                    else
                    {
                        //同步帧低电平
                        DEBUG_SENDER(0xFC);
                        ASK_SENDER_PIN = 0;
                        AskSender.time_couter = ASK_SENDER_DELAY_SYNC;
                        AskSender.frame_bit_sent++;
                    }
                    DEBUG_SENDER(0xFF);
                }
                else
                {
                    DEBUG_SENDER(0x01);
                    //先发的高电平
                    ASK_SENDER_PIN = 1;
                    AskSender.bit_switcher = 1;
                    AskSender.time_couter = ASK_SENDER_DELAY_LONG;
                }
            }
        }
    }
}

#endif // ASK_SENDER
