#include "cmd.hpp"
#include <stdlib.h>
#include "linux/input.h"
#include <limits.h>
#include <fcntl.h> // open
#include <string.h> // strerror
#include <errno.h>
#include <stdio.h> 
#include <unistd.h> // daemon, close
#include "eth.h"
#include "video_s.hpp"
#define DEBUG_CMD printf

int cmdSendConfigBuffor(Network * connection)
{
    unsigned char temp_buff = CMD_CONFIG_IMAGE_BUFFOR;
    return connection->write_tcp(connection, (unsigned char*) &temp_buff, 1, 100);
}

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
        if (ev.value == 1)
        switch (ev.code)
        {
            case KEY_ENTER:
            DEBUG_CMD("CMD: send CMD_START_IMG\n");
            buff[0] = CMD_START_IMG;
            buff[1] = 255;
            connection->write_tcp(connection,buff,1,100);
            break;

            case KEY_S:
            DEBUG_CMD("CMD: send CMD_STOP_IMG\n");
            buff[0] = CMD_STOP_IMG;
            buff[1] = 255;
            connection->write_tcp(connection,buff,1,100);
            break;

            case KEY_I:
            DEBUG_CMD("CMD: send CMD_UDP_IP\n");
            buff[0] = CMD_UDP_IP;
            buff[1] = 255;
            connection->write_tcp(connection,buff,1,100);
            connection->write_udp(connection,buff,1);
            break;
            default:
            buff[1] = direction_parse((uint32_t*)(&ev.value), &ev.code, &key_value);
            break;
        }
        
        if(buff[1] < 255) // function can return -1
        {
            buff[0] = CMD_GO;
            connection->write_tcp(connection,buff,2,100);
        }
    }

    if (ev.code == KEY_ESC) break;
    }
    close(fd);
    fflush(stdout);
}

extern video_streaming_c *v_Stream_p;

void * read_cmd_thd(void * network)
{
    Network * connection = (Network *)network;
    char read_tcpBuff[16];
    int len = 0;
    cmd_img_t parameters_img;
    while(1)
    {
        len = connection->read_tcp(connection, (unsigned char*)read_tcpBuff, sizeof(parameters_img) + 1, 100);
        if (len > 0)
        {
            switch(read_tcpBuff[0])
            {
                case CMD_UDP_IP:
                DEBUG_CMD("CMD_UDP_IP\n");
                
                break;
                case CMD_ERROR:
                DEBUG_CMD("CMD_ERROR: %d\n",read_tcpBuff[1]);
                
                break;
                case CMD_CONFIG_IMAGE_BUFFOR:
                DEBUG_CMD("CMD: CMD_CONFIG_IMAGE_BUFFOR\n");
                if (v_Stream_p->state == VID_INIT)
                {
                    memcpy(&parameters_img, &read_tcpBuff[0], sizeof(parameters_img));
                    DEBUG_CMD("CMD: img h=%d, w=%d\n", parameters_img.height, parameters_img.width);
                    v_Stream_p->video_init_buffer(parameters_img.width, parameters_img.height);
                }
                #if 0 //test without keyboard
                DEBUG_CMD("CMD: Start test\n");
                read_tcpBuff[0] = CMD_UDP_IP;
                read_tcpBuff[1] = 255;
                connection->write_tcp(connection,(unsigned char *)read_tcpBuff,1,100);
                connection->write_udp(connection,(unsigned char *)read_tcpBuff,1);
                sleep(2);
                DEBUG_CMD("CMD: Start img\n");
                read_tcpBuff[0] = CMD_START_IMG;
                read_tcpBuff[1] = 255;
                connection->write_tcp(connection,(unsigned char *)read_tcpBuff,1,100);
                #endif
                break;
            }
        }
    }   
}
#define RX_IMG_BUFF_LEN 1024
unsigned char receive_buffor[RX_IMG_BUFF_LEN];
void * receive_img_thd(void * network)
{
    Network * connection = (Network *)network;
    int len_addr = sizeof(connection->servaddr_udp);
    packetUDP *packet = (packetUDP *)(receive_buffor);
    while(1)
    {
        if (v_Stream_p->state == VID_READY)
        {
            if (connection->read_udp(connection, receive_buffor, RX_IMG_BUFF_LEN, &len_addr) > 0)
            {
              
                memcpy(&v_Stream_p->video_buff[v_Stream_p->number_img%2][packet->number_packet*(RX_IMG_BUFF_LEN - sizeof(packetUDP))], 
                    &receive_buffor[sizeof(packetUDP) - 1], RX_IMG_BUFF_LEN - sizeof(packetUDP));
                if (packet->number_packet == v_Stream_p->size_video_buff/(RX_IMG_BUFF_LEN - sizeof(packetUDP)))
                {
                    //DEBUG_CMD("CMD: read udp data %d \n", v_Stream_p->number_img%2);
                    v_Stream_p->show_image();
                }
            }
            
        }
    }
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
