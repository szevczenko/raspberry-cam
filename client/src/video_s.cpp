#include "linux/input.h"
#include <limits.h>
#include <pthread.h>
#include <semaphore.h>
#include "video_s.hpp"
#include "unistd.h"
#include "cmd.hpp"
#include "eth.h"
#include "robot_param.hpp"

using namespace std;
using namespace cv;

#define C_WIDTH 600
#define C_HEIGHT 480
#define VIDEO_BUFF_LEN 3
#define DEBUG_VIDEO printf

sem_t video_sem;

video_streaming_c::video_streaming_c(size_t width, size_t height)
{
    if (state == VID_NO_INIT)
    {
        namedWindow("stream", CV_WINDOW_AUTOSIZE);
        sem_init(&video_sem, 0, 1);
        video_buff[0] = new char[width * height];
        video_buff[1] = new char[width * height];
        size_video_buff = width * height;
        img[0] = Mat(width, height, CV_8UC1, video_buff[0]);
        img[1] = Mat(width, height, CV_8UC1, video_buff[1]);
        size = PTHREAD_STACK_MIN;
        DEBUG_VIDEO("size = %lu\n",size);
        errno = pthread_attr_init(&attr);
	    if (errno) {
		    perror("pthread_attr_init");
	    }
        state = VID_READY;
        type = TYPE_VID_CAMERA;
	    //pthread_attr_getstacksize(&attr, &size);
        //pthread_create(&video_s, &attr, video_receive_thd, (void *) this);//
    }
}

void video_streaming_c::wait_end(void)
{
    pthread_join(video_s, NULL);
}

video_streaming_c::video_streaming_c(char * buff, char * buff2, size_t width, size_t height)
{
    if (state == VID_NO_INIT)
    {
        namedWindow("stream", CV_WINDOW_AUTOSIZE);
        sem_init(&video_sem, 0, 1);
        video_buff[0] = buff;
        video_buff[1] = buff2;
        img[0] = Mat(width, height, CV_8UC1, video_buff[0]);
        img[1] = Mat(width, height, CV_8UC1, video_buff[1]);
        size_video_buff = width * height;
        size = PTHREAD_STACK_MIN;
        DEBUG_VIDEO("size = %lu\n",size);
        errno = pthread_attr_init(&attr);
	    if (errno) {
		    perror("pthread_attr_init");
	    }
        state = VID_READY;
        type = TYPE_VID_CAMERA;
	    //pthread_attr_getstacksize(&attr, &size);
        //pthread_create(&video_s, &attr, video_receive_thd, this);
    }
}

video_streaming_c::video_streaming_c(void)
{
    if (state == VID_NO_INIT)
    {
        namedWindow("stream", CV_WINDOW_AUTOSIZE);
        size = PTHREAD_STACK_MIN;
        DEBUG_VIDEO("VIDEO: void constructor\n");
        errno = pthread_attr_init(&attr);
	    if (errno) {
		    perror("pthread_attr_init");
	    }
        state = VID_INIT;
        type = TYPE_VID_CAMERA;
	    //pthread_attr_getstacksize(&attr, &size);
        //pthread_create(&video_s, &attr, video_receive_thd, this);
    }
}

void video_streaming_c::video_init_buffer(size_t width_p, size_t height_p)
{
    if (state == VID_INIT)
    {
        sem_init(&video_sem, 0, 0);
        width = width_p;
        height = height_p;
        video_buff[0] = new char[width * height];
        video_buff[1] = new char[width * height];
        size_video_buff = width * height;
        img[0] = Mat(height, width, CV_8UC1, video_buff[0]);
        img[1] = Mat(height, width, CV_8UC1, video_buff[1]);
        state = VID_READY;
        type = TYPE_VID_CAMERA;
    }
    
}

video_streaming_c::~video_streaming_c(void)
{
    pthread_exit(&video_s);
    delete video_buff[0];
    delete video_buff[1];
}

int video_streaming_c::show_image(void)
{
    if (state == VID_READY)
    {
        number_img++;
        return sem_post(&video_sem);
    }
    else return -1;
    
}

extern Network scon;

void video_receive_thd(void * pv)
{
    video_streaming_c *v_stream = static_cast<video_streaming_c*>(pv);
    Mat *img[2];
    img[0] = &v_stream->img[0];
    img[1] = &v_stream->img[1];
    DEBUG_VIDEO("VIDEO: start process \n");
    int ret_val = 0;
    size_t capture;
    timespec timeout;
    timeout.tv_nsec = 100000;
    timeout.tv_sec = 0;
    test_robot();
    //v_stream->video_init_buffer(800, 600);
    while(1)
    {
        if (v_stream->state == VID_READY && v_stream->type == TYPE_VID_CAMERA)
        {
            
            if (sem_timedwait(&video_sem, &timeout) != 0) continue;
            capture = (v_stream->number_img - 1)%2;
            //DEBUG_VIDEO("VIDEO: Semaphore activated capture = %lu\n", capture);
            if(img[capture]->empty())
            {
                DEBUG_VIDEO("VIDEO: Img empty\n");
            }
            imshow("stream", *img[capture]);
            waitKey(5);
        }
        else if (v_stream->state != VID_READY && v_stream->type == TYPE_VID_CAMERA )
        {
            ret_val = cmdSendConfigBuffor(&scon);
            if (ret_val<0)  printf("VIDEO: send buff (%s)\n", strerror(errno));
            DEBUG_VIDEO("VIDEO: send msg config buffor. ret = %d \n", ret_val);
            sleep(1);
        }
        else if (v_stream->type == TYPE_VID_POSITION)
        {
            img[0]->setTo(Scalar(0));
            config_visualization(img[0], &config_robot);
            imshow("stream", *img[0]);
            waitKey(100);
        }
        
    }
}