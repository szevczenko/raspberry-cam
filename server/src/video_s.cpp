#include "linux/input.h"
#include <limits.h>
#include <pthread.h>
#include <semaphore.h>
#include "unistd.h"

#include "video_s.hpp"

using namespace std;
//using namespace cv;

#define C_WIDTH 320
#define C_HEIGHT 240
#define VIDEO_BUFF_LEN 3
#define DEBUG_VIDEO printf
#define PACKET_SIZE 1024

video_streaming_c::video_streaming_c(void)
{
    buffor_size = C_WIDTH * C_HEIGHT;
    data_len_send = PACKET_SIZE;
    buffor = new char[buffor_size];
    packet_count = buffor_size/(data_len_send - sizeof(packetUDP)) + 1;
    memset(&cliaddr, 0, sizeof(cliaddr));
    //tets
    for (uint32_t i = 0; i<buffor_size; i++)
    {
        buffor[i] = 50;
    }
}
video_streaming_c::~video_streaming_c(void)
{
    delete buffor;
}

static void video_thread(video_streaming_c * stream)
{
    char packet[PACKET_SIZE];
    packetUDP header;
    while(1)
    {
        if (stream->state != VID_SEND_DATA)
        {
            sleep(1);
            continue;
        }
        header.packet_len = stream->data_len_send - sizeof(packetUDP);
        //DEBUG_VIDEO("VIDEO: send packet UDP \n");
        for(uint32_t i = 0; i<stream->packet_count; i++)
        {
            header.number_packet = i;
            if (i == stream->packet_count - 1)
            {
                header.packet_len = stream->data_len_send - sizeof(packetUDP) - \
                    (stream->buffor_size - (stream->data_len_send - sizeof(packetUDP))* (stream->packet_count - 1));
            }
            memcpy(packet, &header, sizeof(header));
            memcpy(&packet[sizeof(header) - 1], &stream->buffor[i*(stream->data_len_send - sizeof(packetUDP))], header.packet_len);
            sendto(stream->socket, packet, stream->data_len_send,  
                MSG_CONFIRM, (const struct sockaddr *) &stream->cliaddr, 
                    stream->len_addr);
        }
        //test
        for(uint32_t i = 0; i<stream->buffor_size; i++)
        {
            stream->buffor[i] = stream->buffor[i] + 25;
        }
        usleep(100000);
    }
     
}

int video_streaming_c::start_process(void)
{
    th = thread(video_thread,this);
    return 1;
}

int video_streaming_c::wait_to_end(void)
{
    th.join();
    return 1;
}
