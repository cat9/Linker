#include "SimpleBeep.h"

#define BEEP_STATE_ON 1
#define BEEP_STATE_OFF 0

#define BEEP_TIMER_UNIT_MS 10

#define BEEP_PIN_ON 1
#define BEEP_PIN_OFF 0

void SimpleBeep_Start(const SimpleBeep *simpleBeep,u8 beepTime10us, u8 stopTime10us, u8 beepCount)
{
    simpleBeep->busy = 0;
    simpleBeep->write_pin_value(BEEP_PIN_OFF);
    simpleBeep->beep_time = beepTime10us;
    simpleBeep->beep_stop_time = stopTime10us;
    simpleBeep->beep_time_now = 0;
    simpleBeep->stop_time_now = 0;
    simpleBeep->beep_count = beepCount;
    simpleBeep->beep_count_now = 0;
    simpleBeep->state_switch = BEEP_STATE_ON;
    simpleBeep->level_switch = 0;
    simpleBeep->beep_time_counter = 0;
    simpleBeep->busy = 1;
}

void SimpleBeep_Stop(const SimpleBeep *simpleBeep)
{
    simpleBeep->busy = 0;
    simpleBeep->write_pin_value(BEEP_PIN_OFF);
}

void SimpleBeep_HeartBeat(const SimpleBeep *simpleBeep)
{
    if (simpleBeep->busy)
    {
        simpleBeep->beep_time_counter++;
        if (simpleBeep->state_switch == BEEP_STATE_ON)
        {
            // 鸣叫
            if (simpleBeep->level_switch == 0)
            {
                simpleBeep->level_switch = 1;
                simpleBeep->write_pin_value(BEEP_PIN_ON);
            }
            else
            {
                simpleBeep->level_switch = 0;
                simpleBeep->write_pin_value(BEEP_PIN_OFF);
            }
            if(simpleBeep->beep_time_counter>BEEP_TIMER_UNIT_MS){
                simpleBeep->beep_time_counter = 0;
                if(++simpleBeep->beep_time_now > simpleBeep->beep_time){
                    simpleBeep->beep_time_now = 0;
                    simpleBeep->state_switch = BEEP_STATE_OFF;
                }
            }
        }
        else
        {
            // 暂停
            if(simpleBeep->beep_time_counter>=BEEP_TIMER_UNIT_MS){
                simpleBeep->beep_time_counter = 0;
                if(++simpleBeep->beep_time_now > simpleBeep->beep_stop_time){
                    simpleBeep->beep_time_now = 0;
                    if(simpleBeep->beep_count>0){
                        //有次数限制
                        if(++simpleBeep->beep_count_now >= simpleBeep->beep_count){
                            simpleBeep->beep_count_now = 0;
                            simpleBeep->busy = 0;
                            simpleBeep->write_pin_value(BEEP_PIN_OFF);
                        }else{
                            simpleBeep->state_switch = BEEP_STATE_ON;
                        }
                    }else{
                        //无次数限制
                        simpleBeep->state_switch = BEEP_STATE_ON;
                    }
                }
            }else{
                simpleBeep->write_pin_value(BEEP_PIN_OFF);
            }
        }
    }
}