#ifndef __SIMPLEKEY_H__
#define __SIMPLEKEY_H__

#include "../config.h"



typedef struct SimpleKey
{
    bit (*read_pin_value)();
    u8 pressTime;
    u8 clickedAutoConsumer;
    u8 isKeyDown:1;
    u8 flipFlag:1;
    u8 busy:1;
} xdata SimpleKey;

u8 SimpleKey_IsClicked(const SimpleKey *simpleKey);
u8 SimpleKey_IsLongPressed(const SimpleKey *simpleKey);
void SimpleKey_Consumed(const SimpleKey *simpleKey);
u8 SimpleKey_IsBusy(const SimpleKey *simpleKey);
void SimpleKey_HeartBeat(const SimpleKey *simpleKey);


#endif // __SIMPLEKEY_H__


