#ifndef _CMD_H
#define _CMD_H

#define KEY_UP_POS 0
#define KEY_DOWN_POS 1
#define KEY_LEFT_POS 2
#define KEY_RIGHT_POS 3

#include <stdint.h>
#include "eth.h"

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
    CMD_UDP_CONFIRM,
    CMD_START_IMG,
    CMD_STOP_IMG,
    CMD_CONFIG_IMAGE_BUFFOR,
    CMD_ERROR
}NET_CMD;

typedef struct
{
    char cmd;
    uint16_t height;
    uint16_t width;
}cmd_img_t;

typedef struct
{
    uint32_t number_packet;
    uint32_t packet_len;   
}packetUDP;

extern void * keyboard_thd(void * network);
int direction_parse(uint32_t * ev_value, uint16_t * ev_code, uint8_t * key_value);
int cmdSendConfigBuffor(Network * connection);
void * read_cmd_thd(void * network);
void * receive_img_thd(void * network);

#endif