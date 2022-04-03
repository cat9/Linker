
#include "Test.h"
#include "config.h"
#include "ASK.h"
#include "UART.h"


u8 buffer_receive[16];

void TestInit(){
     Ask_ReceiverInit();
}

void TestRun(){
    u8 i=Ask_ReceiverRead(buffer_receive,3);
    if(i>0)
    {
        LOGB(0xcc);
        LOGB(i);
        for (i = 0; i < 3; i++)
        {
            LOGB(buffer_receive[i]);
        }
    }
    LOGB(0xee);
}