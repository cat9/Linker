#ifndef __ASK_H__
#define __ASK_H__

#include "config.h"

//开关
#define ASK_SENDER 1
#define ASK_RECEIVER 1

/*---------------------------------------------------------------------*/
/*   目前实现的是固定长度的异步收发                                      */
/*---------------------------------------------------------------------*/


#define ASK_FRAME_SIZE 3  //每帧数据的字节长度，每次发送/接收一帧数据


/*======================================================================================*/
/*======================================================================================*/
/*======================================================================================*/



#ifdef ASK_RECEIVER

    //使用PCA实现的接收功能
    #define ASK_RECEIVER_PIN  P33  //接收端口 
    #define ASK_CCAPL CCAP1L  //接收端口对应的CCAPnL寄存器 低位
    #define ASK_CCAPH CCAP1H  //接收端口对应的CCAPnH寄存器 高位
    #define ASK_CCAPM CCAPM1  //接收端口对应的CCAPMn寄存器 模式位

    #define ASK_RECEIVER_BUFFER_LENGTH 64    // 发送器缓存区字节数量
    

void Ask_ReceiverInit(void);

u8 Ask_ReceiverRead(u8 *dat,u8 len);

#endif // ASK_RECEIVER



/*======================================================================================*/
/*======================================================================================*/
/*======================================================================================*/


#ifdef ASK_SENDER  //ASK发送器

    #define ASK_SENDER_PIN  P32  //发送端口
    #define ASK_SENDER_BUFFER_LENGTH 64    // 发送器缓存区字节数量
    #define ASK_SENDER_FRAME_BIT_LENGTH (ASK_FRAME_SIZE*8) // 发送器每帧数据的位长度=ASK_FRAME_SIZE*8

    //协议波形配置(定时器为50us)
    #define ASK_SENDER_DELAY_LONG 24 // 1.2ms
    #define ASK_SENDER_DELAY_SHORT 8 // 400us
    #define ASK_SENDER_DELAY_SYNC 96 // 4.8ms


    void Ask_SenderInit(void);

    void Ask_Send(u8 dat);

    void Ask_SendBytes(u8 buff[], u8 len);

    void Ask_SenderProcess();

#endif // ASK_SENDER

#endif // __ASK_H__
