#include "cmd.h"
#include "stdio.h"
#include "string.h"
#include <stdint.h>
#include <config.h>

#if !CONFIG_PLATFORM_LINUX
#include <wiringPi.h>
#include <softPwm.h>
#endif

#include <semaphore.h>
#include <stdlib.h>
#include "motor_pwm.h"

#define DEBUG_CMD printf

static int pwm_parse1, pwm_parse2;

int parse_cmd(uint8_t * buffer, uint8_t len)
{
    int rv = 0;
    #if !CONFIG_PLATFORM_LINUX
    if (buffer[0]==CMD_GO)
    {
        switch(buffer[1])
        {
            case (GO_STR):
            DEBUG_CMD("GO_STR");
            pwm_parse1 = 80;
            pwm_parse2 = 80;
            break;
            case (GO_BACK):
            DEBUG_CMD("GO_BACK");
            pwm_parse1 = -80;
            pwm_parse2 = -80;
            break;
            case (GO_LEFT):
            DEBUG_CMD("GO_LEFT");
            pwm_parse1 = -80;
            pwm_parse2 = 80;
            break;
            case (GO_RIGHT):
            DEBUG_CMD("GO_RIGHT");
            pwm_parse1 = 80;
            pwm_parse2 = -80;
            break;
            case (GO_STR_LEFT):
            DEBUG_CMD("GO_STR_LEFT");
            pwm_parse1 = 0;
            pwm_parse2 = 80;
            break;
            case (GO_BACK_RIGHT):
            DEBUG_CMD("GO_BACK_RIGHT");
            pwm_parse1 = -80;
            pwm_parse2 = 0;
            break;
            case (GO_BACK_LEFT):
            DEBUG_CMD("GO_BACK_LEFT");
            pwm_parse1 = 0;
            pwm_parse2 = -80;
            break;
            case (GO_STR_RIGHT):
            DEBUG_CMD("GO_STR_RIGHT");
            pwm_parse1 = 80;
            pwm_parse2 = 0;
            break;
            case (GO_STOP):
            DEBUG_CMD("GO_STOP");
            pwm_parse1 = 0;
            pwm_parse2 = 0;
            break;
            default:
            DEBUG_CMD("Invalid cmd");
            rv = -1;
            break; 
        }
        DEBUG_CMD("\n");
    }
    #else
    DEBUG_CMD("receive %d %d \n", buffer[0], buffer[1]);
    #endif //#if !CONFIG_PLATFORM_LINUX
    if (0);
    else rv = -1;
    #if !CONFIG_PLATFORM_LINUX
    if (rv>-1)  sem_post (&sem_go);
    #endif //#if !CONFIG_PLATFORM_LINUX
    return rv;
    
}

#if !CONFIG_PLATFORM_LINUX
void * go_cmd_thd(void * pv)
{
    DEBUG_CMD("thd go_cmd start\n");
    while(1)
    {
        sem_wait (&sem_go);
        go_str(pwm_parse1, pwm_parse2);
    }
    
}
#endif
