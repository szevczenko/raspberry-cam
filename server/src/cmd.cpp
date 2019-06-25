#include "cmd.hpp"
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
#include "motor_pwm.hpp"
#include "video_s.hpp"

#define DEBUG_CMD printf

extern video_streaming_c *vStreamObj_pnt;

static int pwm_parse1, pwm_parse2;
typedef struct
{
    char cmd;
    uint16_t height;
    uint16_t width;
}cmd_img_t;

static cmd_img_t buff_cmd_img;

int parse_cmd(uint8_t * buffer, uint8_t len, int client_socket)
{
    int rv = 0;
    char buffer_to_send[8];
    switch(buffer[0])
    {
        case CMD_UDP_IP:

        break;
        case CMD_START_IMG:
        if (vStreamObj_pnt->cliaddr.sin_addr.s_addr != 0)
        {
            vStreamObj_pnt->state = VID_SEND_DATA;
            vStreamObj_pnt->socket_tcp = client_socket;
        }
        else 
        {
            DEBUG_CMD("CMD: error CMD_START_IMG\n");
            buffer_to_send[0] = CMD_ERROR;
            buffer_to_send[1] = CMD_START_IMG;
            write(client_socket, &buffer_to_send, 2);
        }
        break;
        case CMD_STOP_IMG:
            vStreamObj_pnt->state = VID_STOP_SEND;
        break;
        case CMD_CONFIG_IMAGE_BUFFOR:
        DEBUG_CMD("CMD: send config image buffor\n");
        buff_cmd_img.cmd = CMD_CONFIG_IMAGE_BUFFOR;
        buff_cmd_img.height = CAM_HEIGHT;
        buff_cmd_img.width = CAM_WIDTH;
        write(client_socket, &buff_cmd_img, sizeof(buff_cmd_img));
        break;
    }
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
        #if !CONFIG_PLATFORM_LINUX
        if (rv>-1)  sem_post (&sem_go);
        #endif //#if !CONFIG_PLATFORM_LINUX
    }
    #else
    DEBUG_CMD("receive %d %d \n", buffer[0], buffer[1]);
    #endif //#if !CONFIG_PLATFORM_LINUX
    if (0);
    else rv = -1;
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
