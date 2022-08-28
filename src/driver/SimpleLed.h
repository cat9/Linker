#ifndef __SIMPLE_LED_H__
#define __SIMPLE_LED_H__

#include "../config.h"

typedef struct SimpleLed
{
    void (*write_pin_value)(const u8 value);
    u8 on_time;
    u8 off_time;
    u8 on_time_now;
    u8 off_time_now;
    u8 triger_count;
    u8 triger_count_now;
    u8 time_counter;
    u8 state_switch:1;
    u8 level_switch:1;
    u8 busy:1;
} xdata SimpleLed;

void SimpleLed_Start(const SimpleLed *simpleLed, u8 onTime, u8 offTime, u8 trigerCount);
void SimpleLed_Stop(const SimpleLed *simpleLed);

void SimpleLed_HeartBeat(const SimpleLed *simpleLed);


#endif // !__SIMPLE_LED_H__