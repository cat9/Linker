#include "SimpleLed.h"

#define BEEP_TIMER_UNIT_MS 100

#define LED_ON 1
#define LED_OFF 0

void SimpleLed_Start(const SimpleLed *simpleLed, u8 onTimeBy100ms, u8 offTimeBy100ms, u8 trigerCount){
    simpleLed->busy = 0;
    simpleLed->write_pin_value(LED_OFF);
    simpleLed->level_switch = LED_OFF;
    simpleLed->on_time = onTimeBy100ms;
    simpleLed->off_time = offTimeBy100ms;
    simpleLed->on_time_now = 0;
    simpleLed->off_time_now = 0;
    simpleLed->triger_count = trigerCount;
    simpleLed->triger_count_now = 0;
    simpleLed->state_switch = LED_ON;
    simpleLed->time_counter = 0;
    simpleLed->busy = 1;
}


void SimpleLed_Stop(const SimpleLed *simpleLed){
    simpleLed->busy = 0;
    simpleLed->write_pin_value(LED_OFF);
}

void SimpleLed_HeartBeat(const SimpleLed *simpleLed){
     if (simpleLed->busy)
    {
        simpleLed->time_counter++;
        if (simpleLed->state_switch == LED_ON)
        {
            // 点亮
            if (simpleLed->level_switch == LED_OFF)
            {
                simpleLed->write_pin_value(LED_ON);
                simpleLed->level_switch = LED_ON;
            }
            if(simpleLed->time_counter>BEEP_TIMER_UNIT_MS){
                simpleLed->time_counter = 0;
                if(++simpleLed->on_time_now > simpleLed->on_time){
                    simpleLed->on_time_now = 0;
                    simpleLed->state_switch = LED_OFF;
                }
            }
        }
        else
        {
            // 暂停
            if(simpleLed->time_counter>=BEEP_TIMER_UNIT_MS){
                simpleLed->time_counter = 0;
                if(++simpleLed->on_time_now > simpleLed->off_time){
                    simpleLed->on_time_now = 0;
                    if(simpleLed->triger_count>0){
                        //有次数限制
                        if(++simpleLed->triger_count_now >= simpleLed->triger_count){
                            simpleLed->triger_count_now = 0;
                            simpleLed->busy = 0;
                            simpleLed->write_pin_value(LED_OFF);
                            simpleLed->level_switch = LED_OFF;
                        }else{
                            simpleLed->state_switch = LED_ON;
                        }
                    }else{
                        //无次数限制
                        simpleLed->state_switch = LED_ON;
                    }
                }
            }else{
                simpleLed->write_pin_value(LED_OFF);
                simpleLed->level_switch = LED_OFF;
            }
        }
    }

}