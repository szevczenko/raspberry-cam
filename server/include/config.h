#ifndef _CONFIG_H
#define _CONFIG_H

#define PWM0_NUMBER         0
#define PWM1_NUMBER         1
#define PWM_PIN_1_STR       0
#define PWM_PIN_1_BACK      2
#define PWM_PIN_2_STR       1
#define PWM_PIN_2_BACK      3
#define DEF_PWM_MAX         100
#define DEF_PWM_TURN_STR_0  100
#define DEF_PWM_TURN_STR_1  25
#define MS_STOP             10
#define MAX_VALUE(VAL1,VAL2) (VAL1 >= VAL2)?VAL1:VAL2
#define delay_ms(ms)        usleep(ms*1000)
#define DEBUGF printf

#define CONFIG_PLATFORM_LINUX 1
#endif