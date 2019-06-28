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
#include "camera.hpp"

#define DEBUG_CMD printf
extern piCamera * obCamPnt;
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
        case CMD_START_LOCALIZATION:
            if (obCamPnt->type != CAM_LOCALIZATION)
            {
                obCamPnt->deinit();
                obCamPnt->init(600, 600, 0, CAM_LOCALIZATION);
            }
            obCamPnt->startCam();
        break;
        case CMD_STOP_LOCALIZATION:
            obCamPnt->stopCam();
        break;
    
        case CMD_UDP_IP:

        break;
        case CMD_START_IMG:
            if (obCamPnt->type != CAM_STREAM)
            {
                obCamPnt->deinit();
                obCamPnt->init(CAM_WIDTH, CAM_HEIGHT, (unsigned char *)vStreamObj_pnt->buffor, CAM_STREAM);
            }
            obCamPnt->startCam();
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
            obCamPnt->stopCam();
        break;
        case CMD_CONFIG_IMAGE_BUFFOR:
        DEBUG_CMD("CMD: send config image buffor\n");
        buff_cmd_img.cmd = CMD_CONFIG_IMAGE_BUFFOR;
        buff_cmd_img.height = CAM_HEIGHT;
        buff_cmd_img.width = CAM_WIDTH;
        write(client_socket, &buff_cmd_img, sizeof(buff_cmd_img));
        break;

        #if !CONFIG_PLATFORM_LINUX
        case CMD_GO:        
        switch(buffer[1])
        {
            case (GO_STR):
            DEBUG_CMD("GO_STR\n");
            pwm_parse1 = 80;
            pwm_parse2 = 80;
            break;
            case (GO_BACK):
            DEBUG_CMD("GO_BACK\n");
            pwm_parse1 = -80;
            pwm_parse2 = -80;
            break;
            case (GO_LEFT):
            DEBUG_CMD("GO_LEFT\n");
            pwm_parse1 = -80;
            pwm_parse2 = 80;
            break;
            case (GO_RIGHT):
            DEBUG_CMD("GO_RIGHT\n");
            pwm_parse1 = 80;
            pwm_parse2 = -80;
            break;
            case (GO_STR_LEFT):
            DEBUG_CMD("GO_STR_LEFT\n");
            pwm_parse1 = 0;
            pwm_parse2 = 80;
            break;
            case (GO_BACK_RIGHT):
            DEBUG_CMD("GO_BACK_RIGHT\n");
            pwm_parse1 = -80;
            pwm_parse2 = 0;
            break;
            case (GO_BACK_LEFT):
            DEBUG_CMD("GO_BACK_LEFT\n");
            pwm_parse1 = 0;
            pwm_parse2 = -80;
            break;
            case (GO_STR_RIGHT):
            DEBUG_CMD("GO_STR_RIGHT\n");
            pwm_parse1 = 80;
            pwm_parse2 = 0;
            break;
            case (GO_STOP):
            DEBUG_CMD("GO_STOP\n");
            pwm_parse1 = 0;
            pwm_parse2 = 0;
            break;
            default:
            DEBUG_CMD("Invalid cmd\n");
            rv = -1;
            break;
        }
        break;
       
        #endif
    }
    if (rv>-1)  
        {
            sem_post (&sem_go);
        }
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
