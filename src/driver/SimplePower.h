#ifndef __SIMPLEPOWER_H__
#define __SIMPLEPOWER_H__

#include "../config.h"

#define LVDF 0x20
#define LVD2V0 0x00
#define LVD2V4 0x01
#define LVD2V7 0x02
#define LVD3V0 0x03
#define LVDFULL 0x04

typedef struct SimplePower {
    void (*on_power_changed)(void);
    void (*test_config)(struct SimplePower *simplePower,u8 index);
    u8 (*test_voltage)(struct SimplePower *simplePower,u8 index);
    u8 power_last;
    u8 power_now;
    u8 power_low;
    u8 time_counter;
    u8 time_period;
    u8 test_index;
    u8 test_step:1; //0:配置，1：测试
    u8 test_end_index;
    u8 test_end_index_last;
} xdata SimplePower;


void SimplePower_Init(SimplePower* simplePower, u8 lowPowerVoltage,u8 highPowerVoltage,u8 testTimePeriod);

void SimplePower_HeartBeat(SimplePower* simplePower);



#endif // !__SIMPLEPOWER_H__