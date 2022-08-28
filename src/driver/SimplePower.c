#include "SimplePower.h"



void SimplePower_HeartBeat(SimplePower* simplePower){
    if(++simplePower->time_counter>=simplePower->time_period){
        simplePower->time_counter=0;
        if(simplePower->test_step){
            //测试
            if(simplePower->test_voltage(simplePower,simplePower->test_index)){
                if(simplePower->test_index==0){
                    simplePower->test_end_index=0;
                }else{
                    simplePower->test_index--;
                }
            }else{
                simplePower->test_end_index=simplePower->test_index+1;
            }
            simplePower->test_step=0;
        }else{
            //配置
            simplePower->test_config(simplePower,simplePower->test_index);
            simplePower->test_step=1;
        }
    }
}