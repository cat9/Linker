#include "App.h"
#include "Task.h"
#include "ASK.h"
#include "UART.h"

sbit LED = P5 ^ 5;

sbit OUT_PIN = P3 ^ 2;

u8 buffer[16];
u8 i;


void App_Config()
{

    for (i = 0; i < 16; i++)
    {
        buffer[i] = 0X61;
    }
    for (i = 0; i < 16; i++)
    {
        LOGB(buffer[i]);
    }

    // CCON = 0x00;
    // CMOD = 0x00;
    // CL=0x00;
    // CH=0x00;
    // CCAP1L = 0x00;
    // CCAP1H = 0x00;
    // CCAPM1 = 0x31;
    // CR = 1;
    // EA = 1;

    // Ask_ReceiverInit();

}

// void PCA_Interrupt() interrupt 7{
//     if (CF)
//     {
//         CF=0;
//         LOGB(0xbb);
//     }

//     if (CCF1)
//     {
//         CCF1 = 0;
        
//         CR = 0;
//         LOGB(0xff);
//         LOGB(CCAP1H);
//         LOGB(0xaa);
//         LOGB(CCAP1L);
//         CL=0x00;
//         CH=0x00;
//         CCAP1L = 0x00;
//         CCAP1H = 0x00;
//         CR = 1;
//     }
// }


void App_Run()
{
    // OUT_PIN=~OUT_PIN;
    LED = ~LED;
    // LED = ~LED;
    Ask_SendBytes(buffer, 3);
    Ask_SendBytes(buffer, 3);
    Ask_SendBytes(buffer, 3);
    Ask_SendBytes(buffer, 3);
    // for (i = 0; i < 3; i++)
    // {
    //     LOGB(buffer[i]);
    // }


}
