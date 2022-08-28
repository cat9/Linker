#ifndef __KEY_H__
#define __KEY_H__

#include "config.h"


void Key_Init();

u8 Key_IsClicked();

u8 Key_IsLongPressed();

void Key_Consumed();

u8 Key_IsBusy();

void Key_Detect();

#endif // !__





