#include "App.h"
#include "Task.h"
#include "ASK.h"
#include "UART.h"
#include "Device.h"
#include "Key.h"

#define LED P55

u8 i;

KeyDefine buttonDef;

void App_Config()
{
    Device_Init();
    Key_Init(&buttonDef, P32);

    for (i = 0; i < 16; i++)
    {
        buffer[i] = 0X61;
    }
    for (i = 0; i < 16; i++)
    {
        LOGB(buffer[i]);
    }

}


u8 buffer_receive[ASK_FRAME_SIZE];
u8 read;
void App_Run()
{
    
    read=Ask_ReceiverRead(buffer_receive,ASK_FRAME_SIZE);
    if(read==ASK_FRAME_SIZE)
    {
        for (i = 0; i < ASK_FRAME_SIZE; i++)
        {
            LOGB(buffer_receive[i]);
        }
    }
    LED = ~LED;
}