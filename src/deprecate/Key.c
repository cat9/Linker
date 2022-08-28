#include "Key.h"
#include "UART.h"

#define KEY_PIN P32

#define KEY_CLICK_TIME_MIN 8
#define KEY_CLICK_TIME_MAX 120

#define KEY_LONG_PRESS_TIME 254

u8 pressTime;
bit consumed;
u8 isKeyDown;
bit flipFlag;
u8 clickedAutoConsumer;

void Key_Init()
{
    isKeyDown = 0;
    pressTime = 0;
    consumed = 1;
    flipFlag = 0;
    clickedAutoConsumer = 0;
}

u8 Key_IsClicked()
{
    if (consumed == 0 && isKeyDown == 0 && pressTime > KEY_CLICK_TIME_MIN && pressTime < KEY_CLICK_TIME_MAX)
    {
        return 1;
    }
    return 0;
}

u8 Key_IsLongPressed()
{
    if (consumed == 0 && pressTime > KEY_LONG_PRESS_TIME)
    {
        return 1;
    }
    return 0;
}

void Key_Consumed()
{
    consumed = 1;
}

u8 Key_IsBusy()
{
    return !consumed;
}

void Key_Detect()
{
    if (KEY_PIN == 0)
    {
        if (isKeyDown == 0)
        {
            isKeyDown = 1;
            pressTime = 0;
            consumed = 0;
            clickedAutoConsumer = 0;
        }
        else
        {
            flipFlag = ~flipFlag;
            if (flipFlag == 0 && pressTime < 255)
            {
                pressTime++;
            }
        }
    }
    else
    {
        if (isKeyDown == 1)
        {
            isKeyDown = 0;
            // LOGB(pressTime);
            if (pressTime > KEY_LONG_PRESS_TIME)
            {
                consumed = 1;
                pressTime = 0;
            }
            else if (pressTime > KEY_CLICK_TIME_MIN && pressTime < KEY_CLICK_TIME_MAX)
            {
                //自动延时消费掉事件，否则可能会误触发之前的点击事件
                clickedAutoConsumer = 0Xff;
            }
        }
        else
        {
            if (consumed == 0)
            {
                //自动延时消费掉事件，否则可能会误触发之前的点击事件
                if (clickedAutoConsumer > 0)
                {
                    clickedAutoConsumer--;
                }
                if (clickedAutoConsumer == 0)
                {
                    consumed = 1;
                    pressTime = 0;
                    // LOGB(0x22);
                }
            }
        }
    }
}
