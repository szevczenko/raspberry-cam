#include "linux/input.h"
#include <limits.h>
#include <pthread.h>
#include <semaphore.h>
#include "video_s.hpp"

using namespace std;
using namespace cv;

#define C_WIDTH 600
#define C_HEIGHT 480
#define VIDEO_BUFF_LEN 3
#define DEBUG_VIDEO printf

sem_t video_sem;

video_streaming_c::video_streaming_c(size_t width, size_t height)
{
    namedWindow("stream", CV_WINDOW_AUTOSIZE);
    sem_init(&video_sem, 0, 1);
    video_buff = new char[width * height];
    img = Mat(width, height, CV_8UC1, video_buff);
    size = PTHREAD_STACK_MIN;
    DEBUG_VIDEO("size = %lu\n",size);
    errno = pthread_attr_init(&attr);
	if (errno) {
		perror("pthread_attr_init");
	}
	pthread_attr_getstacksize(&attr, &size);
    pthread_create(&video_s, &attr, video_receive_thd, (void *) &img);//
    
}

void video_streaming_c::wait_end(void)
{
    pthread_join(video_s, NULL);
}

video_streaming_c::video_streaming_c(char * buff, size_t width, size_t height)
{
    namedWindow("stream", CV_WINDOW_AUTOSIZE);
    sem_init(&video_sem, 0, 1);
    video_buff = buff;
    img = Mat(width, height, CV_8UC1, video_buff);
    size = PTHREAD_STACK_MIN;
    DEBUG_VIDEO("size = %lu\n",size);
    errno = pthread_attr_init(&attr);
	if (errno) {
		perror("pthread_attr_init");
	}
	pthread_attr_getstacksize(&attr, &size);
    pthread_create(&video_s, &attr, video_receive_thd, &img);
}
video_streaming_c::~video_streaming_c(void)
{
    pthread_exit(&video_s);
    delete video_buff;
}

int video_streaming_c::show_image(void)
{
    return sem_post(&video_sem);
}


void * video_receive_thd(void * pv)
{
    Mat *img = static_cast<Mat*>(pv);
    DEBUG_VIDEO("VIDEO: start process \n");
    while(1)
    {
        sem_wait (&video_sem);
        imshow("stream", *img);
        waitKey(5);
    }
}