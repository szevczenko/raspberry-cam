#include "cmd.hpp"
#include "stdio.h"
#include "string.h"
#include <stdint.h>
#include <config.h>

#if !CONFIG_PLATFORM_LINUX

#include <wiringPi.h>
#include <softPwm.h>
#include <stdlib.h>
#include <motor_pwm.hpp>

struct motor_pwm command;
sem_t sem_go;

void init_pwm(void)
{
    command.pwm_max = DEF_PWM_MAX;
    command.pwm_turn_str[0] = DEF_PWM_MAX;
    command.ms_stop = MS_STOP;
    memset(command.pwm1_to_do, 0,sizeof(command.pwm1_to_do));
    memset(command.pwm2_to_do, 0,sizeof(command.pwm2_to_do));
    command.previos_pwm[0] = 0;
    command.acceleration = 2;
    command.pwm[0] = 0;
    command.pin_pwm1 = PWM_PIN_1_STR;
    command.pin_pwm2 = PWM_PIN_2_STR;
    if (wiringPiSetup () == -1)
	{
		DEBUGF("wiringPiSetup error\n");
		exit (1) ;
	}
    softPwmCreate(PWM_PIN_1_STR,0,100);
	softPwmCreate(PWM_PIN_2_STR,0,100);
    softPwmCreate(PWM_PIN_1_BACK,0,100);
	softPwmCreate(PWM_PIN_2_BACK,0,100);
    sem_init(&sem_go, 0, 1);
}

void change_turn(uint8_t numb_pwm)
{
    if (numb_pwm == PWM0_NUMBER)
    {
        if (command.pin_pwm1 == PWM_PIN_1_STR) command.pin_pwm1 = PWM_PIN_1_BACK;
        else command.pin_pwm1 = PWM_PIN_1_STR;
    }
    else
    {
        if (command.pin_pwm2 == PWM_PIN_2_STR) command.pin_pwm2 = PWM_PIN_2_BACK;
        else command.pin_pwm2 = PWM_PIN_2_STR;
    }
}

int insert_toDo1(int pwm1);
int insert_toDo2(int pwm2);
int do_toDo1(void);
int do_toDo2(void);


void go_str(int pwm1, int pwm2)
{
    // if have same sign1
    int sign1 = 1;
    int sign2 = 1;
    int retv1 = 0, retv2 = 0;
    insert_toDo1(pwm1);
    insert_toDo2(pwm2);
    do
	{
        if(retv1 == 0) retv1 = do_toDo1();
        if(retv2 == 0) retv2 = do_toDo2();
        if(retv1 && retv2) break;
		delay_ms(command.acceleration);
	}while(1);
    //DEBUGF("end do\n");
    if (pwm1<0 && command.pwm[0] >= 0) sign1 = -1;
    command.previos_pwm[0] = command.pwm[0]*sign1;
    if (pwm2<0 && command.pwm[1] >= 0) sign2 = -1;
    command.previos_pwm[1] = command.pwm[1]*sign2;
    //DEBUGF("return = %d, pwm = %d, sign1 = %d, pin = %d\n",command.previos_pwm[0], command.pwm[0], sign1, command.pin_pwm1);
    //command.previos_pwm[1] = command.pwm[1];
}

int insert_toDo1(int pwm1)
{
    command.to_do_cnt1 = 0;
    if((command.previos_pwm[0] >= 0 && pwm1 >= 0) || (command.previos_pwm[0] < 0 && pwm1 < 0))
    {

        if (pwm1>=0) command.pwm1_to_do[command.to_do_cnt1].step = pwm1 - command.previos_pwm[0];
        else command.pwm1_to_do[command.to_do_cnt1].step = -pwm1 + command.previos_pwm[0];

        if (command.pwm1_to_do[command.to_do_cnt1].step >= 0)
        {
            command.pwm1_to_do[command.to_do_cnt1].toDo = PWM_INCREMENT;
        }
        else 
        {
            command.pwm1_to_do[command.to_do_cnt1].toDo = PWM_DECREMENT;
            command.pwm1_to_do[command.to_do_cnt1].step = -command.pwm1_to_do[command.to_do_cnt1].step;
        }

        command.to_do_cnt1++;
    }
    else //Not same
    {
        command.pwm1_to_do[command.to_do_cnt1].toDo = PWM_STOP;
        command.pwm1_to_do[command.to_do_cnt1].step = command.ms_stop;
        command.to_do_cnt1++;
        command.pwm1_to_do[command.to_do_cnt1].toDo = PWM_CHANGE;
        command.pwm1_to_do[command.to_do_cnt1].step = 1;
        command.to_do_cnt1++;

        if (pwm1>=0) 
        {
            command.pwm1_to_do[command.to_do_cnt1].step = pwm1;
            command.pwm1_to_do[command.to_do_cnt1].toDo = PWM_INCREMENT;
        }
        else 
        {
            command.pwm1_to_do[command.to_do_cnt1].step = -pwm1;
            command.pwm1_to_do[command.to_do_cnt1].toDo = PWM_INCREMENT;
        }
        command.to_do_cnt1++;
    }
    command.to_do_cnt1=0;
    return 1;
}

int insert_toDo2(int pwm2)
{
    command.to_do_cnt2 = 0;
    if((command.previos_pwm[1] >= 0 && pwm2 >= 0) || (command.previos_pwm[1] < 0 && pwm2 < 0))
    {

        if (pwm2>=0) command.pwm2_to_do[command.to_do_cnt2].step = pwm2 - command.previos_pwm[1];
        else command.pwm2_to_do[command.to_do_cnt2].step = -pwm2 + command.previos_pwm[1];

        if (command.pwm2_to_do[command.to_do_cnt2].step >= 0)
        {
            command.pwm2_to_do[command.to_do_cnt2].toDo = PWM_INCREMENT;
        }
        else 
        {
            command.pwm2_to_do[command.to_do_cnt2].toDo = PWM_DECREMENT;
            command.pwm2_to_do[command.to_do_cnt2].step = -command.pwm2_to_do[command.to_do_cnt2].step;
        }

        command.to_do_cnt2++;
    }
    else //Not same
    {
        command.pwm2_to_do[command.to_do_cnt2].toDo = PWM_STOP;
        command.pwm2_to_do[command.to_do_cnt2].step = command.ms_stop;
        command.to_do_cnt2++;
        command.pwm2_to_do[command.to_do_cnt2].toDo = PWM_CHANGE;
        command.pwm2_to_do[command.to_do_cnt2].step = 1;
        command.to_do_cnt2++;

        if (pwm2>=0) 
        {
            command.pwm2_to_do[command.to_do_cnt2].step = pwm2;
            command.pwm2_to_do[command.to_do_cnt2].toDo = PWM_INCREMENT;
        }
        else 
        {
            command.pwm2_to_do[command.to_do_cnt2].step = -pwm2;
            command.pwm2_to_do[command.to_do_cnt2].toDo = PWM_INCREMENT;
        }
        command.to_do_cnt2++;
    }
    command.to_do_cnt2=0;
    return 1;
}

int do_toDo1(void)
{
    if (command.pwm1_to_do[command.to_do_cnt1].step > 0)
    {
        //DEBUGF("step = %d, cmd = %d, cnt = %d\n",command.pwm1_to_do[command.to_do_cnt1].step,command.pwm1_to_do[command.to_do_cnt1].toDo, command.to_do_cnt1);
        switch (command.pwm1_to_do[command.to_do_cnt1].toDo)
        {
            case (PWM_STOP):
            command.pwm1_to_do[command.to_do_cnt1].toDo = PWM_NOTHING;
            command.pwm[0] = 0;
            softPwmWrite(command.pin_pwm1,command.pwm[0]);
            break;
            case (PWM_INCREMENT):
            command.pwm[0]++;
            softPwmWrite(command.pin_pwm1,command.pwm[0]);
            break;
            case (PWM_DECREMENT):
            command.pwm[0]--;
            softPwmWrite(command.pin_pwm1,command.pwm[0]);
            break;
            case (PWM_CHANGE):
            change_turn(PWM0_NUMBER);
            break;
            case (PWM_NOTHING):
            break;
            case (PWM_EMPTY):
            break;
        }
        command.pwm1_to_do[command.to_do_cnt1].step--;
    }
    else
    {
        //DEBUGF("ex: step = %d, cmd = %d, cnt = %d\n",command.pwm1_to_do[command.to_do_cnt1].step,command.pwm1_to_do[command.to_do_cnt1].toDo, command.to_do_cnt1);
        if (command.pwm1_to_do[command.to_do_cnt1].toDo == PWM_EMPTY)
        {
          memset(command.pwm1_to_do, 0, sizeof(command.pwm1_to_do));
          return 1;
        }
        command.to_do_cnt1++;
    }
    return 0;
}

int do_toDo2(void)
{
    if (command.pwm2_to_do[command.to_do_cnt2].step > 0)
    {
        //DEBUGF("step = %d, cmd = %d, cnt = %d\n",command.pwm1_to_do[command.to_do_cnt1].step,command.pwm1_to_do[command.to_do_cnt1].toDo, command.to_do_cnt1);
        switch (command.pwm2_to_do[command.to_do_cnt2].toDo)
        {
            case (PWM_STOP):
            command.pwm2_to_do[command.to_do_cnt2].toDo = PWM_NOTHING;
            command.pwm[1] = 0;
            softPwmWrite(command.pin_pwm2,command.pwm[1]);
            break;
            case (PWM_INCREMENT):
            command.pwm[1]++;
            softPwmWrite(command.pin_pwm2,command.pwm[1]);
            break;
            case (PWM_DECREMENT):
            command.pwm[1]--;
            softPwmWrite(command.pin_pwm2,command.pwm[1]);
            break;
            case (PWM_CHANGE):
            change_turn(PWM1_NUMBER);
            break;
            case (PWM_NOTHING):
            break;
            case (PWM_EMPTY):
            break;
        }
        command.pwm2_to_do[command.to_do_cnt2].step--;
    }
    else
    {
        //DEBUGF("ex: step = %d, cmd = %d, cnt = %d\n",command.pwm1_to_do[command.to_do_cnt1].step,command.pwm1_to_do[command.to_do_cnt1].toDo, command.to_do_cnt1);
        if (command.pwm2_to_do[command.to_do_cnt2].toDo == PWM_EMPTY)
        {
          memset(command.pwm2_to_do, 0, sizeof(command.pwm2_to_do));
          return 2;
        }
        command.to_do_cnt2++;
    }
    return 0;
}

#endif