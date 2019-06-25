#ifndef _MOTOR_PWM_
#define _MOTOR_PWM_

#include <semaphore.h>

struct pwm_to_do
{
    uint8_t toDo;
    int step;
};

struct motor_pwm
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

void init_pwm(void);
void go_str(int pwm1, int pwm2);

extern sem_t sem_go;

#endif