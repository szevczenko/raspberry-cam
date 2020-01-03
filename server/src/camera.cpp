#include "config.h"
#include <ctime>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include "camera.hpp"
#include "video_s.hpp"
#include "v_analyse.hpp"
#include "robot_param.hpp"
#include "cmd.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/xfeatures2d.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/video/tracking.hpp"
#include <chrono>
//#include "opencv2/cvstd.hpp"

#define DEBUG_CAM printf

void treshold(unsigned char * data, unsigned char * dst_data, uint32_t data_size, uint8_t max_value);

sem_t sem_find_obj;
pthread_t	find_thd;
ledClass led_object;

static void * find_process(void * pv);

void v_analyse_init_find_process(void)
{
    sem_init(&sem_find_obj, 0, 1);
}

void start_find_process(pthread_attr_t	*attr, piCamera * cam_obj)
{
    pthread_create(&find_thd, attr, find_process, (void*)cam_obj);
}

void wait_to_end_find(void)
{
    pthread_join(find_thd, NULL);
}

static void * find_process(void * pv)
{
    piCamera * cam_obj = (piCamera *) pv;
    while(1)
    {
        sem_wait (&sem_find_obj);
        treshold(cam_obj->grey_data, cam_obj->post_process_data, cam_obj->img_size/3, 225);
        for (uint32_t y = 0; y < cam_obj->cam_height; y++)
        {
            for (uint32_t x = 0; x < cam_obj->cam_width; x++)
            {
                if (go(cam_obj->post_process_data, &led_object, cam_obj->cam_width, x, y) == 1)
                {
                    led_object.count_average();
                    if (led_object.count_pixel > 6)
                    {
                        led_object.check();
                    }
                }

            }
        }
        led_object.post_process(&cam_obj->color_img);
    }

}


using namespace cv::xfeatures2d;

piCamera::piCamera(void)
{
    state = CAM_NO_INIT;
    type = CAM_NO_PROCESS;
    cam_width = 0;
    cam_height = 0;
    img_size = 0;
}

int piCamera::stopCam(void)
{
    if (state == CAM_START)
    {
        state = CAM_STOP;
        return 1;
    }
    else
    {
        DEBUG_CAM("CAM: Stop error. Camera not started\n");
        return -1;
    }
}

int piCamera::startCam(void)
{
    if (state == CAM_STOP || state == CAM_INIT)
    {
        state = CAM_START;
        DEBUG_CAM("CAM: Camera start\n");
        return 1;
    }
    else
    {
        DEBUG_CAM("CAM: Start error. Camera not started\n");
        return -1;
    }
}

static uint32_t frame_cnt;

int piCamera::init(int width, int height, unsigned char* data, int type_p)
{
    if (state != CAM_NO_INIT && state != CAM_DEINIT)
    {
        DEBUG_CAM("CAM: Cam was initialized\n");
        return -1;
    }
    if (type_p == CAM_STREAM && data == 0)
    {
        DEBUG_CAM("CAM: ERROR cam stream init without buff\n");
        return -1;
    }
    Camera.setWidth(width);
    Camera.setHeight(height);
    cam_width = width;
    cam_height = height;
    img_size = width*height*3;
    Camera.setRotation(180);
    Camera.setFrameRate(30);
    if ( !Camera.open()) {cerr<<"Error opening camera"<<endl;return -1;}
    //namedWindow("New Window", CV_WINDOW_AUTOSIZE);
    if (data == 0)
    {
        grey_data = new unsigned char[ width*height ];
        post_process_data = new unsigned char[ width*height ];
        DEBUG_CAM("CAM: dynamic alocated memory for grey_data\n");
    }
    else
    {
        grey_data = data;
    }

    color_data = new unsigned char[  Camera.getImageTypeSize ( RASPICAM_FORMAT_RGB )];
    if (grey_data == 0 || color_data == 0 || post_process_data == 0)
    {
        DEBUG_CAM("CAM: ERROR cannot alocate memory\n");
        return -1;
    }
    color_img = Mat(height, width, CV_8UC3, color_data);
    grey_img = Mat(height, width, CV_8UC1, grey_data);
    post_process_img = Mat(height, width, CV_8UC1, post_process_data);

    type = type_p;
    DEBUG_CAM("CAM: Camera init ");
    switch(type)
    {
        case CAM_STREAM:
        DEBUG_CAM("CAM_STREAM\n");
        break;
        case CAM_AUTO_DRIVE:
        case CAM_LOCALIZATION:
        DEBUG_CAM("CAM_AUTO_DRIVE\n");
        break;
    }
    state = CAM_INIT;
    return 1;

}

void piCamera::deinit(void)
{
    if (state == CAM_NO_INIT || state == CAM_DEINIT)
        return;
    state = CAM_NO_INIT;
    cam_width = 0;
    cam_height = 0;
    img_size = 0;
    delete color_data;
    if (type != CAM_STREAM)
    {
        delete grey_data;
        delete post_process_data;
    }
    Camera.release();
    DEBUG_CAM("CAM: Camera deInit \n");
}

void treshold(unsigned char * data, unsigned char * dst_data, uint32_t data_size, uint8_t max_value)
{
    for (uint32_t i=0; i < data_size; i++)
    {
        if (data[i] < max_value) dst_data[i] = 0;
        else dst_data[i] = data[i];
    }
}

void piCamera::find_object(void)
{

    for (uint32_t y = 0; y < cam_height; y++)
    {
        for (uint32_t x = 0; x < cam_width; x++)
        {
            if (go(post_process_data, &led_object, cam_width, x, y) == 1)
            {
                led_object.count_average();
                if (led_object.count_pixel > 10)
                {
                    led_object.check();
                }
            }



        }
    }
    led_object.post_process(&color_img);
}

void piCamera::send_position(void)
{
    uint8_t buff_to_send[256];
    buff_to_send[0] = CMD_POSITION_DATA;
    memcpy(&buff_to_send[1], &config_robot, sizeof(config_robot));
    write(socket_tcp, &buff_to_send, sizeof(config_robot) + 1);
    
}

void piCamera::process(void)
{
    DEBUG_CAM("CAM: Star cam process. Cam type == %d\n", type);
    Mat traj = Mat::zeros(600, 600, CV_8UC3);
    int scaling_draw = 0;
    while(1)
    {
        if (state != CAM_START)
        {
            sleep(1);
            //DEBUG_CAM("CAM: Cam not start, sleep\n");
            continue;
        }
        //capture
        Camera.grab();
        //extract the image in rgb format
        Camera.retrieve ( color_data,RASPICAM_FORMAT_IGNORE );//get camera image

        for (uint32_t i = 0; i < img_size; i++)
        {
            if (i%3 == 0)
            {
                grey_data[i/3] = (color_data[i] + color_data[i-1] + color_data[i-2])/3;

            }
        }
        frame_cnt++;
        if (type == CAM_STREAM && frame_cnt%2==0)
        {
            sem_post (&sem_img_ready);
            //printf("CAM: frame %d\n", frame_cnt);
            //imshow("Grey_img",grey_img);
        }
        if (type == CAM_AUTO_DRIVE || type == CAM_LOCALIZATION)
        {
            sem_post (&sem_find_obj);

            if (scaling_draw % 3 == 0)
            {
                led_object.draw_object(&color_img);
                imshow("New Window",color_img);
                waitKey(1);
            }
            scaling_draw++;
        }
    }
}

