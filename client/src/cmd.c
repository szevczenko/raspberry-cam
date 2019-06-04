#include "cmd.h"
#include <stdlib.h>
#include "linux/input.h"
#include <limits.h>
#include <fcntl.h> // open
#include <string.h> // strerror
#include <errno.h>
#include <stdio.h> 
#include <unistd.h> // daemon, close
#include "eth.h"

void * keyboard_thd(void * network)
{
    Network * connection = (Network *)network;
    const char *dev = "/dev/input/by-path/platform-i8042-serio-0-event-kbd";
    int fd;
    uint8_t buff[16];
    char read_tcpBuff[16];
    fd = open(dev, O_RDONLY); // Open the buffer
    printf("errno (%s)\n",strerror(errno) );
        if (fd == -1) return NULL;
 
    struct input_event ev;
    ssize_t n;
    uint8_t key_value = 0;

    while (1) {
    n = read(fd, &ev, sizeof ev); // read_tcp from the buffer
    
    if (ev.type == EV_KEY)
    {
        buff[1] = direction_parse(&ev.value, &ev.code, &key_value);
        if(buff[1] < 255) // function can return -1
        {
            buff[0] = CMD_GO;
            connection->write_tcp(connection,buff,2,100);
            connection->write_udp(connection,"Hello", 5); //test
        }
    }

    if (ev.code == KEY_ESC) break;
    }
    close(fd);
    fflush(stdout);
}

int direction_parse(uint32_t * ev_value, uint16_t * ev_code, uint8_t * key_value)
{
    //printf("code = %d value = %d\n", *ev_code, *ev_value);
     if (*ev_value == 1) //value 0 == released; 1 == pressed; 2 == repeated
    {
       // printf("Key %d has been pressed\n", ev.code); // up - 103, down - 108, left - 105, right 106.
        switch(*ev_code)
        {
            case KEY_UP:
            *key_value |= (1<<KEY_UP_POS);
            break;
            case KEY_DOWN:
            *key_value |= (1<<KEY_DOWN_POS);
            break;
            case KEY_LEFT:
            *key_value |= (1<<KEY_LEFT_POS);
            break;
            case KEY_RIGHT:
            *key_value |= (1<<KEY_RIGHT_POS);
            break;
            default:
            break; 
        }
    }
    else if(*ev_value == 0)
    {
        switch(*ev_code)
        {
            case KEY_UP:
            *key_value &= ~(1<<KEY_UP_POS);
            break;
            case KEY_DOWN:
            *key_value &= ~(1<<KEY_DOWN_POS);
            break;
            case KEY_LEFT:
            *key_value &= ~(1<<KEY_LEFT_POS);
            break;
            case KEY_RIGHT:
            *key_value &= ~(1<<KEY_RIGHT_POS);
            break;
            default:
            break; 
        }
    }

    if (*ev_value != 2)
    {
        switch(*key_value)
        {
            case (1<<KEY_UP_POS):
            return GO_STR;
            break;
            case (1<<KEY_DOWN_POS):
            return GO_BACK;
            break;
            case (1<<KEY_LEFT_POS):
            return GO_LEFT;
            break;
            case (1<<KEY_RIGHT_POS):
            return GO_RIGHT;
            break;
            case (1<<KEY_UP_POS)|(1<<KEY_LEFT_POS):
            return GO_STR_LEFT;
            break;
            case (1<<KEY_DOWN_POS)|(1<<KEY_RIGHT_POS):
            return GO_BACK_RIGHT;
            break;
            case (1<<KEY_LEFT_POS)|(1<<KEY_DOWN_POS):
            return GO_BACK_LEFT;
            break;
            case (1<<KEY_RIGHT_POS)|(1<<KEY_UP_POS):
            return GO_STR_RIGHT;
            break;
            default:
            return GO_STOP;
            break; 
        }
    }
    return -1;
}
