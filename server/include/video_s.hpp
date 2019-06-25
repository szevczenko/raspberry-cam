#ifndef _VIDEO_S_HPP
#define _VIDEO_S_HPP

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/select.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <thread> 

using namespace std;
using namespace cv;

typedef struct
{
    uint32_t number_packet;
    uint32_t packet_len;   
}packetUDP;

typedef enum 
{
    VID_NO_INIT = 0,
    VID_INIT,
    VID_READY,
    VID_SEND_DATA,
    VID_STOP_SEND
}vidStateEn;


class video_streaming_c {
  public:
  video_streaming_c(void);
  ~video_streaming_c(void);
  vidStateEn state;
  int socket;
  int socket_tcp; //for test who receive image
  struct sockaddr_in cliaddr;
  int len_addr;
  int send_data(char *data, size_t data_len);
  int receive_data(char *data, size_t data_len);
  int init_client(video_streaming_c client);
  int start_process(void);
  int wait_to_end(void);
  int packet_count;
  size_t data_len_send; 
  char *buffor;
  size_t buffor_size;
  private:
  thread th;
  
};


#endif