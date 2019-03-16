#ifndef _CMD_H
#define _CMD_H

#define KEY_UP_POS 0
#define KEY_DOWN_POS 1
#define KEY_LEFT_POS 2
#define KEY_RIGHT_POS 3

#include <stdint.h>
#include "thd.h"

typedef enum
{
    GO_STOP = 0,
    GO_STR ,
    GO_STR_LEFT,
    GO_LEFT,
    GO_BACK_LEFT,
    GO_BACK,
    GO_BACK_RIGHT,
    GO_RIGHT,
    GO_STR_RIGHT,   
}MOTION;

typedef enum
{
    CMD_GO = 0
}NET_CMD;

struct pwm_to_do
{
    uint8_t toDo;
    int step;
};

struct do_cmd
{
    int pwm_max;
    int pwm_turn_str[2];
    int acceleration;
    int previos_pwm[2];
    int pwm[2];
    int new_pwm[2];
    uint16_t ms_stop;
    struct pwm_to_do pwm1_to_do[16];
    struct pwm_to_do pwm2_to_do[16];
    uint8_t to_do_cnt1;
    uint8_t to_do_cnt2;
    uint8_t pin_pwm1;
    uint8_t pin_pwm2;

};

typedef enum
{
    PWM_EMPTY = 0,
    PWM_NOTHING,
    PWM_STOP,
    PWM_CHANGE,
    PWM_INCREMENT,
    PWM_DECREMENT
}PWM_DO; 
int parse_cmd(uint8_t * buffer, uint8_t len);
void init_pwm(void);
void go_str(int pwm1, int pwm2);
void * go_cmd_thd(void * pv);
//void * keyboard_thd(void * network);
//int direction_parse(uint32_t * ev_value, uint16_t * ev_code, uint8_t * key_value);


#endif