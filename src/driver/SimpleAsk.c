#include "SimpleAsk.h"
#include "UART.h"
#include "../utils/Memery.h"

// #include "stdio.h"

#ifdef SIMPLE_ASK_RECEIVER

// #define DEBUG_RECEIVER(BYTE) TX1_write2buff(BYTE)
#define DEBUG_RECEIVER(BYTE)


void SimpleAskReceiver_Init(SimpleAskReceiver *simpleAskReceiver,u8 busy_reload,u8 repeat_reload,u16 short_time_us,u16 long_time_us)
{
    simpleAskReceiver->repeat_counter = repeat_reload;
    simpleAskReceiver->repeat_reload = repeat_reload;

    simpleAskReceiver->busy_counter = busy_reload;
    simpleAskReceiver->busy_reload = busy_reload;

    simpleAskReceiver->receive_ping_value = 0;

    simpleAskReceiver->receive_hight_flag = 0;     //高电平长短标志 0:短 1:长
    simpleAskReceiver->receive_low_flag = 0;       //低电平长短标志 0:短 1:长
    simpleAskReceiver->receive_hight_flag_set = 0; //高电平是否接收完成标志 0:未完成 1:完成
    simpleAskReceiver->receive_low_flag_set = 0;   //低电平是否接收完成标志 0:未完成 1:完成

    simpleAskReceiver->receive_bit_count = 0; //当前收到的bit数量
    simpleAskReceiver->receive_byte = 0;      //当前收到的byte

    simpleAskReceiver->receive_buff_write_index = 0; //缓冲区写索引
    simpleAskReceiver->receive_buff_read_index = 0;  //缓冲区读索引
    simpleAskReceiver->receive_buff_frame_index = 0; //接收帧缓冲区索引

    simpleAskReceiver->signal_length = 0; //电平长度
    simpleAskReceiver->now_value = 0;     //当前中断值
    simpleAskReceiver->last_value = 0;    //上次中断值

    //信号70%
    simpleAskReceiver->bound_short_delay_lower = short_time_us - short_time_us * 3 / 10;
    simpleAskReceiver->bound_short_delay_upper = short_time_us + short_time_us * 3 / 10;
    simpleAskReceiver->bound_long_delay_lower = long_time_us - long_time_us * 3 / 10;
    simpleAskReceiver->bound_long_delay_upper = long_time_us + long_time_us * 3 / 10;
}

u8 SimpleAskReceiver_Read(SimpleAskReceiver *simpleAskReceiver,u8 *buff, u8 len)
{
    u8 i;
    // LOGB(0xfa);
    // LOGB(0xc1);
    // LOGB(simpleAskReceiver->receive_buff_read_index);
    // LOGB(simpleAskReceiver->receive_buff_write_index);
    for (i = 0; i < len && simpleAskReceiver->receive_buff_read_index != simpleAskReceiver->receive_buff_write_index; i++)
    {
        buff[i] = simpleAskReceiver->receiver_buffer[simpleAskReceiver->receive_buff_read_index++];
        // DEBUG_RECEIVER(buff[i]);
        if (simpleAskReceiver->receive_buff_read_index >= ASK_RECEIVER_BUFFER_LENGTH)
        {
            simpleAskReceiver->receive_buff_read_index = 0;
        }
    }
    // LOGB(i);
    // LOGB(0xfe);
    return i;
}

u8 check_equal(const u8 *p, const u8 *q, u8 len)
{
    while (len > 0)
    {
        if (*p != *q)
        {
            return 0;
        }
        p++;
        q++;
        len--;
    }
    return 1;
}

void SimpleAskReceiver_Process(SimpleAskReceiver *simpleAskReceiver)
{
    simpleAskReceiver->receive_hight_flag_set = 0;
    simpleAskReceiver->receive_low_flag_set = 0;
    DEBUG_RECEIVER(0xfd);
    if (simpleAskReceiver->receive_hight_flag)
    {
        DEBUG_RECEIVER(0x99);
    }
    else
    {
        DEBUG_RECEIVER(0xaa);
    }
    if (simpleAskReceiver->receive_low_flag)
    {
        DEBUG_RECEIVER(0x99);
    }
    else
    {
        DEBUG_RECEIVER(0xaa);
    }

    if (simpleAskReceiver->receive_hight_flag ^ simpleAskReceiver->receive_low_flag)
    {
        simpleAskReceiver->receive_byte <<= 1;
        if (simpleAskReceiver->receive_hight_flag)
        {
            //高长低短 bit=1
            simpleAskReceiver->receive_byte |= 0x01;
        }
        else
        {
            //高短低长 bit=0
            simpleAskReceiver->receive_byte &= 0xfe;
        }
        simpleAskReceiver->receive_bit_count++;
        DEBUG_RECEIVER(simpleAskReceiver->receive_bit_count);
    }
    else
    {
        DEBUG_RECEIVER(0xf4);
        simpleAskReceiver->receive_hight_flag_set = 0;
        simpleAskReceiver->receive_low_flag_set = 0;

        simpleAskReceiver->receive_byte = 0;
        simpleAskReceiver->receive_bit_count = 0;
        simpleAskReceiver->receive_buff_frame_index = 0;
        return;
    }

    if (simpleAskReceiver->receive_bit_count > 7)
    {
        //接收完一个byte
        DEBUG_RECEIVER(0xf5);
        simpleAskReceiver->receive_bit_count = 0;
        simpleAskReceiver->frame_buffer[simpleAskReceiver->receive_buff_frame_index] = simpleAskReceiver->receive_byte;
        simpleAskReceiver->receive_buff_frame_index++;
        DEBUG_RECEIVER(simpleAskReceiver->receive_byte);
    }

    if (simpleAskReceiver->receive_buff_frame_index >= ASK_FRAME_SIZE)
    {
        DEBUG_RECEIVER(0xF6);
        //接收完成
        simpleAskReceiver->receive_hight_flag_set = 0;
        simpleAskReceiver->receive_low_flag_set = 0;
        simpleAskReceiver->receive_bit_count = 0;
        if(simpleAskReceiver->repeat_counter>0){
            if(check_equal(simpleAskReceiver->frame_buffer,simpleAskReceiver->frame_buffer_last,ASK_FRAME_SIZE)){
                simpleAskReceiver->repeat_counter=simpleAskReceiver->repeat_reload;
                simpleAskReceiver->receive_buff_frame_index = 0;
                simpleAskReceiver->receive_byte = 0;
                return;
            }
        }

        for (simpleAskReceiver->receive_buff_frame_index = 0; simpleAskReceiver->receive_buff_frame_index < ASK_FRAME_SIZE; simpleAskReceiver->receive_buff_frame_index++)
        {
            simpleAskReceiver->receive_byte = simpleAskReceiver->frame_buffer[simpleAskReceiver->receive_buff_frame_index];
            DEBUG_RECEIVER(simpleAskReceiver->receive_byte);
            simpleAskReceiver->frame_buffer_last[simpleAskReceiver->receive_buff_frame_index] = simpleAskReceiver->receive_byte;
            simpleAskReceiver->receiver_buffer[simpleAskReceiver->receive_buff_write_index++] = simpleAskReceiver->receive_byte;
            if (simpleAskReceiver->receive_buff_write_index >= ASK_RECEIVER_BUFFER_LENGTH)
            {
                simpleAskReceiver->receive_buff_write_index = 0;
            }
        }
        DEBUG_RECEIVER(simpleAskReceiver->receive_buff_read_index);
        DEBUG_RECEIVER(simpleAskReceiver->receive_buff_write_index);
        simpleAskReceiver->repeat_counter=simpleAskReceiver->repeat_reload;
        simpleAskReceiver->receive_buff_frame_index = 0;
        simpleAskReceiver->receive_byte = 0;
    }
}

u8 hi = 0;
u8 low = 0;

void SimpleAskReceiver_HeartBeat(SimpleAskReceiver *simpleAskReceiver){
        DEBUG_RECEIVER(0xBB);
        simpleAskReceiver->busy_counter = simpleAskReceiver->busy_reload;
        // ((u8 *)&simpleAskReceiver->now_value)[0] = ASK_CCAPH;
        // ((u8 *)&simpleAskReceiver->now_value)[1] = ASK_CCAPL;

        simpleAskReceiver->now_value = simpleAskReceiver->read_capture_counter_value();

        if (simpleAskReceiver->now_value > simpleAskReceiver->last_value)
        {
            simpleAskReceiver->signal_length = simpleAskReceiver->now_value - simpleAskReceiver->last_value;
        }
        else
        {
            simpleAskReceiver->signal_length = 65536 - simpleAskReceiver->last_value + simpleAskReceiver->now_value;
        }

        // simpleAskReceiver->receive_ping_value = ASK_RECEIVER_PIN;
        simpleAskReceiver->receive_ping_value = simpleAskReceiver->read_capture_ping_value();
        if (simpleAskReceiver->receive_ping_value)
        {
            DEBUG_RECEIVER(0x01);
        }
        else
        {
            DEBUG_RECEIVER(0x00);
        }

        hi = simpleAskReceiver->now_value / 256;
        low = simpleAskReceiver->now_value % 256;
        DEBUG_RECEIVER(hi);
        DEBUG_RECEIVER(low);

        hi = simpleAskReceiver->last_value / 256;
        low = simpleAskReceiver->last_value % 256;
        DEBUG_RECEIVER(hi);
        DEBUG_RECEIVER(low);

        simpleAskReceiver->last_value = simpleAskReceiver->now_value;

        // simpleAskReceiver->receive_ping_value = ASK_RECEIVER_PIN;
        // if (simpleAskReceiver->receive_ping_value)
        // {
        //     DEBUG_RECEIVER(0x01);
        // }
        // else
        // {
        //     DEBUG_RECEIVER(0x00);
        // }
        hi = simpleAskReceiver->signal_length / 256;
        low = simpleAskReceiver->signal_length % 256;
        DEBUG_RECEIVER(hi);
        DEBUG_RECEIVER(low);

        // DEBUG_RECEIVER(0xfe);

        if (simpleAskReceiver->signal_length > simpleAskReceiver->bound_short_delay_lower && simpleAskReceiver->signal_length < simpleAskReceiver->bound_short_delay_upper)
        {
            //短信号
            DEBUG_RECEIVER(0xfb);
            if (simpleAskReceiver->receive_ping_value)
            {
                //上升沿,之前是低电平
                simpleAskReceiver->receive_low_flag = 0; //低短
                simpleAskReceiver->receive_low_flag_set = 1;
            }
            else
            {
                //下降沿，之前是高电平
                simpleAskReceiver->receive_hight_flag = 0; //高短
                simpleAskReceiver->receive_hight_flag_set = 1;
            }
            if (simpleAskReceiver->receive_hight_flag_set & simpleAskReceiver->receive_low_flag_set)
            {
                SimpleAskReceiver_Process(simpleAskReceiver);
            }
            DEBUG_RECEIVER(0xfe);
        }
        else if (simpleAskReceiver->signal_length > simpleAskReceiver->bound_long_delay_lower && simpleAskReceiver->signal_length < simpleAskReceiver->bound_long_delay_upper)
        {
            //长信号
            DEBUG_RECEIVER(0xfc);
            if (simpleAskReceiver->receive_ping_value)
            {
                //上升沿,之前是低电平
                simpleAskReceiver->receive_low_flag = 1; //低长
                simpleAskReceiver->receive_low_flag_set = 1;
            }
            else
            {
                //下降沿，之前是高电平
                simpleAskReceiver->receive_hight_flag = 1; //高长
                simpleAskReceiver->receive_hight_flag_set = 1;
            }
            if (simpleAskReceiver->receive_hight_flag_set & simpleAskReceiver->receive_low_flag_set)
            {
                SimpleAskReceiver_Process(simpleAskReceiver);
            }
            DEBUG_RECEIVER(0xfe);
        }
        else
        {
            //信号长度不对
            DEBUG_RECEIVER(0xf2);
            simpleAskReceiver->receive_hight_flag_set = 0;
            simpleAskReceiver->receive_low_flag_set = 0;

            simpleAskReceiver->receive_byte = 0;
            simpleAskReceiver->receive_bit_count = 0;
            simpleAskReceiver->receive_buff_frame_index = 0;
        }
}

#endif // SIMPLE_ASK_RECEIVER





/*======================================================================================*/

#ifdef SIMPLE_ASK_SENDER

#define ASK_SENDER_FRAME_BIT_LENGTH (ASK_FRAME_SIZE * 8) // 发送器每帧数据的位长度=ASK_FRAME_SIZE*8

// #define DEBUG_SENDER(BYTE) TX1_write2buff(BYTE)
#define DEBUG_SENDER(BYTE)

void SimpleAskSender_Init(SimpleAskSender *simpleAskSender,u16 heart_beat_us,u16 short_time_us,u16 long_time_us)
{
    simpleAskSender->read = 0;
    simpleAskSender->write = 0;
    simpleAskSender->byte_now = 0;
    simpleAskSender->byte_bit_sent = 0;
    simpleAskSender->frame_bit_sent = 0;
    simpleAskSender->bit_switcher = 0;
    simpleAskSender->b_busy = 0;
    simpleAskSender->time_couter = 0;
    simpleAskSender->delay_short = short_time_us/heart_beat_us;
    simpleAskSender->delay_long = long_time_us/heart_beat_us;
    simpleAskSender->delay_sync = simpleAskSender->delay_long*4;
}

void SimpleAskSender_Send(SimpleAskSender *simpleAskSender,u8 dat)
{
    simpleAskSender->sender_buffer[simpleAskSender->write] = dat; //装发送缓冲
    if (++simpleAskSender->write >= ASK_SENDER_BUFFER_LENGTH)
        simpleAskSender->write = 0;
    if (simpleAskSender->b_busy == 0)
    {
        simpleAskSender->time_couter = 1;
        simpleAskSender->b_busy = 1;
    }
}

void SimpleAskSender_SendBytes(SimpleAskSender *simpleAskSender,u8 buff[], u8 len)
{
    u8 i;
    for (i = 0; i < len; i++)
    {
        SimpleAskSender_Send(simpleAskSender,buff[i]);
    }
}

u8 SimpleAskSender_IsBusy(SimpleAskSender *simpleAskSender){
    return simpleAskSender->b_busy;
}

void SimpleAskSender_HeartBeat(SimpleAskSender *simpleAskSender)
{
    if (simpleAskSender->b_busy)
    {
        simpleAskSender->time_couter--;
        if (simpleAskSender->time_couter == 0)
        {
            DEBUG_SENDER(0xCC);
            if (simpleAskSender->frame_bit_sent < ASK_SENDER_FRAME_BIT_LENGTH)
            {
                //取数据
                if (simpleAskSender->byte_bit_sent == 0 && simpleAskSender->bit_switcher == 0)
                {
                    if (simpleAskSender->read != simpleAskSender->write)
                    {
                        simpleAskSender->byte_now = simpleAskSender->sender_buffer[simpleAskSender->read];
                    }
                    else
                    {
                        //缓冲区已空,等待下次数据
                        return;
                    }
                }

                //发送数据
                if (simpleAskSender->byte_now & 0x80)
                {
                    DEBUG_SENDER(0xAA);
                    //发送1
                    if (simpleAskSender->bit_switcher)
                    {
                        DEBUG_SENDER(0x00);
                        //后发低电平
                        simpleAskSender->write_pin_value(0);
                        simpleAskSender->bit_switcher = 0;
                        simpleAskSender->time_couter = simpleAskSender->delay_short;
                        simpleAskSender->frame_bit_sent++;
                        simpleAskSender->byte_bit_sent++;
                        if (simpleAskSender->byte_bit_sent > 7)
                        {
                            simpleAskSender->byte_bit_sent = 0;
                            if (++simpleAskSender->read >= ASK_SENDER_BUFFER_LENGTH)
                            {
                                simpleAskSender->read = 0;
                            }
                        }
                        else
                        {
                            simpleAskSender->byte_now = simpleAskSender->byte_now << 1;
                        }
                    }
                    else
                    {
                        //先发的高电平
                        DEBUG_SENDER(0x01);
                        simpleAskSender->write_pin_value(1);
                        simpleAskSender->bit_switcher = 1;
                        simpleAskSender->time_couter = simpleAskSender->delay_long;
                    }
                }
                else
                {
                    DEBUG_SENDER(0xBB);
                    //发送0
                    if (simpleAskSender->bit_switcher)
                    {
                        DEBUG_SENDER(0x00);
                        //后发低电平
                        simpleAskSender->write_pin_value(0);
                        simpleAskSender->bit_switcher = 0;
                        simpleAskSender->time_couter = simpleAskSender->delay_long;
                        simpleAskSender->frame_bit_sent++;
                        simpleAskSender->byte_bit_sent++;
                        if (simpleAskSender->byte_bit_sent > 7)
                        {
                            simpleAskSender->byte_bit_sent = 0;
                            if (++simpleAskSender->read >= ASK_SENDER_BUFFER_LENGTH)
                            {
                                simpleAskSender->read = 0;
                            }
                        }
                        else
                        {
                            simpleAskSender->byte_now = simpleAskSender->byte_now << 1;
                        }
                    }
                    else
                    {
                        DEBUG_SENDER(0x01);
                        //先发的高电平
                        simpleAskSender->write_pin_value(1);
                        simpleAskSender->bit_switcher = 1;
                        simpleAskSender->time_couter = simpleAskSender->delay_short;
                    }
                }
            }
            else
            {
                DEBUG_SENDER(0xEE);
                //发送帧结束标志
                if (simpleAskSender->bit_switcher)
                {
                    //后发低电平
                    DEBUG_SENDER(0x00);

                    if (simpleAskSender->frame_bit_sent > ASK_SENDER_FRAME_BIT_LENGTH)
                    {
                        DEBUG_SENDER(0xFB);
                        //这里等待最后的同步帧低电平完成再结束
                        simpleAskSender->bit_switcher = 0;
                        simpleAskSender->frame_bit_sent = 0;
                        simpleAskSender->time_couter = 1;
                        if (simpleAskSender->read == simpleAskSender->write)
                        {
                            DEBUG_SENDER(0xFE);
                            simpleAskSender->b_busy = 0;
                        }
                    }
                    else
                    {
                        //同步帧低电平
                        DEBUG_SENDER(0xFC);
                        simpleAskSender->write_pin_value(0);
                        simpleAskSender->time_couter = simpleAskSender->delay_sync;
                        simpleAskSender->frame_bit_sent++;
                    }
                    DEBUG_SENDER(0xFF);
                }
                else
                {
                    DEBUG_SENDER(0x01);
                    //先发的高电平
                    simpleAskSender->write_pin_value(1);
                    simpleAskSender->bit_switcher = 1;
                    simpleAskSender->time_couter = simpleAskSender->delay_long;
                }
            }
        }
    }
}

#endif // SIMPLE_ASK_SENDER