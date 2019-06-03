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
    CMD_GO = 0,
    CMD_UDP_IP,
}NET_CMD;
 
int parse_cmd(uint8_t * buffer, uint8_t len);

void * go_cmd_thd(void * pv);


#endif