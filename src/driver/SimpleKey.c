#include "SimpleKey.h"
#include "UART.h"

#define KEY_CLICK_TIME_MIN 8
#define KEY_CLICK_TIME_MAX 120

#define KEY_LONG_PRESS_TIME 254


u8 SimpleKey_IsClicked(const SimpleKey *simpleKey){
    if (simpleKey->busy == 1 && simpleKey->isKeyDown == 0 && simpleKey->pressTime > KEY_CLICK_TIME_MIN && simpleKey->pressTime < KEY_CLICK_TIME_MAX)
    {
        return 1;
    }
    return 0;
}


u8 SimpleKey_IsLongPressed(const SimpleKey *simpleKey){
    // LOGB(simpleKey->busy);
    // LOGB(simpleKey->pressTime);
    if (simpleKey->busy == 1 && simpleKey->pressTime > KEY_LONG_PRESS_TIME)
    {
        return 1;
    }
    return 0;
}


void SimpleKey_Consumed(const SimpleKey *simpleKey){
    simpleKey->busy = 0;
}


u8 SimpleKey_IsBusy(const SimpleKey *simpleKey){
    return simpleKey->busy;
}


void SimpleKey_HeartBeat(const SimpleKey *simpleKey){
    bit keyValue = simpleKey->read_pin_value();
    // LOGB(0x77);
    if (keyValue)
    {
        if (simpleKey->isKeyDown == 1)
        {
            simpleKey->isKeyDown = 0;
            // LOGB(simpleKey->pressTime);
            if (simpleKey->pressTime > KEY_LONG_PRESS_TIME)
            {
                simpleKey->busy = 0;
                simpleKey->pressTime = 0;
            }
            else if (simpleKey->pressTime > KEY_CLICK_TIME_MIN && simpleKey->pressTime < KEY_CLICK_TIME_MAX)
            {
                //自动延时消费掉事件，否则可能会误触发之前的点击事件
                simpleKey->clickedAutoConsumer = 0Xff;
                // LOGB(0x44);
            }
            // LOGB(0x43);
        }
        else
        {
            if (simpleKey->busy == 1)
            {
                //自动延时消费掉事件，否则可能会误触发之前的点击事件
                if (simpleKey->clickedAutoConsumer > 0)
                {
                    simpleKey->clickedAutoConsumer--;
                }
                if (simpleKey->clickedAutoConsumer == 0)
                {
                    simpleKey->busy = 0;
                    simpleKey->pressTime = 0;
                    // LOGB(0x22);
                }
            }
            // LOGB(0x23);
        }
    }
    else
    {
        if (simpleKey->isKeyDown == 0)
        {
            simpleKey->isKeyDown = 1;
            simpleKey->pressTime = 0;
            simpleKey->busy = 1;
            simpleKey->clickedAutoConsumer = 0;
            // LOGB(0x32);
        }
        else
        {
            simpleKey->flipFlag = ~simpleKey->flipFlag;
            if (simpleKey->flipFlag == 0 && simpleKey->pressTime < 255)
            {
                simpleKey->pressTime++;
                // LOGB(0x33);
            }
        }
    }
}