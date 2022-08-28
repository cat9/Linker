#ifndef __BEEP_H__
#define __BEEP_H__
#include "../config.h"

typedef struct SimpleBeep
{
    void (*write_pin_value)(const u8 value);
    u8 beep_config;
    u8 beep_bit_index;
    u8 beep_time;
    u8 beep_time_now;
    u8 stop_time_now;
    u8 beep_stop_time;
    u8 beep_count;
    u8 beep_count_now;
    u8 beep_time_counter;
    u8 state_switch:1;
    u8 level_switch:1;
    u8 beep_bit:1;
    u8 busy:1;
} xdata SimpleBeep;

void SimpleBeep_Start(const SimpleBeep *simpleBeep,u8 beepConfig,u8 beepTime10ms, u8 stopTime10ms, u8 beepCount);
void SimpleBeep_Stop(const SimpleBeep *simpleBeep);

void SimpleBeep_HeartBeat(const SimpleBeep *simpleBeep);

#endif // !__