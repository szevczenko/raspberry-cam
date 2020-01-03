
// Client side implementation of UDP client-server model
extern "C"
{
#include "eth.h"
#include "linux/input.h"
#include <limits.h>
#include <pthread.h>
}
#include "cmd.hpp"
#include "video_s.hpp"

using namespace std;

#define MSG_OK 0
#define MSG_ERROR -1
#define MSG_TIMEOUT 1

#define PORT     8080 
#define MAXLINE 1024 
#define SERVER_ADR "192.168.1.2"

pthread_mutex_t mutex_client;
pthread_mutexattr_t mutexattr;
video_streaming_c *v_Stream_p;
video_streaming_c videoStream;

Network scon;

int main() { 
    int ret = MSG_ERROR;
    
    NetworkInit(&scon);
    ret = NetworkConnect(&scon,(char*)SERVER_ADR,PORT);
    if (ret != MSG_OK)
    {
        printf("NetworkConnect: error\r\n");
        //return MSG_ERROR;
    }


	pthread_t	keyboard, read_cmd, receive_img;
	pthread_attr_t	attr;
	size_t size = PTHREAD_STACK_MIN;
	
	pthread_mutexattr_init(&mutexattr);
	pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&mutex_client, &mutexattr);
	errno = pthread_attr_init(&attr);
	printf("size = %lu\n",size);
 	if (errno) {
		perror("pthread_attr_init");
		return EXIT_FAILURE;
	}
	pthread_attr_getstacksize(&attr, &size);
	pthread_create(&keyboard, &attr, keyboard_thd, (void *)&scon);
	pthread_create(&read_cmd, &attr, read_cmd_thd, (void *)&scon);
	videoStream;
	v_Stream_p = &videoStream;
	pthread_create(&receive_img, &attr, receive_img_thd, (void *)&scon);
	video_receive_thd(v_Stream_p);
	videoStream.wait_end();
	pthread_join(keyboard, NULL);
	pthread_join(receive_img, NULL);
	pthread_join(read_cmd, NULL);
	pthread_attr_destroy(&attr);

    return 0;
} 

/////////////////////////////////////////////////////////////////////////////
