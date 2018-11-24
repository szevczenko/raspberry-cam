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

struct do_cmd
{

};
int parse_cmd(uint8_t * buffer, uint8_t len);
//void * keyboard_thd(void * network);
//int direction_parse(uint32_t * ev_value, uint16_t * ev_code, uint8_t * key_value);


#endif