#include "config.h"
#include <ctime>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include "camera.hpp"
#include "video_s.hpp"

#define DEBUG_CAM printf

piCamera::piCamera(void)
{
    state = CAM_NO_INIT;
    type = CAM_STREAM;
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
        return 1;
    }
    else
    {
        DEBUG_CAM("CAM: Start error. Camera not started\n");
        return -1;
    }
}

int piCamera::init(int width, int height, unsigned char* data, int type)
{
    if (state != CAM_NO_INIT && state != CAM_DEINIT)
    {
        DEBUG_CAM("CAM: Cam was initialized\n");
        return -1;
    }
    if (type == CAM_STREAM && data == 0)
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
    if ( !Camera.open()) {cerr<<"Error opening camera"<<endl;return -1;}
    namedWindow("New Window", CV_WINDOW_AUTOSIZE);
    if (data == 0)
    {
        grey_data = new unsigned char[ width*height ];
        DEBUG_CAM("CAM: dynamic alocated memory for grey_data\n");
    }
    else
    {
        grey_data = data;
    }
    
    color_data = new unsigned char[  Camera.getImageTypeSize ( RASPICAM_FORMAT_RGB )];
    if (grey_data == 0 || color_data == 0)
    {
        DEBUG_CAM("CAM: ERROR cannot alocate memory\n");
        return -1;
    }
    color_img = Mat(height, width, CV_8UC3, color_data);
    grey_img = Mat(height, width, CV_8UC1, grey_data);

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
        delete grey_data;
    Camera.release();
}
void piCamera::process(void)
{

    while(1)
    {
        if (state != CAM_START)
        {
            sleep(1);
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
                /*buff_pix = static_cast<int>(data_background[i/3]) - static_cast<int>(grey_data[i/3]);
                if (buff_pix >= 0)
                    data_output[i/3] = static_cast<unsigned char>(buff_pix);
                else
                    data_output[i/3] = static_cast<unsigned char>((-1)*buff_pix);
                */
            }
        }       
        sem_post (&sem_img_ready); 
        //imshow("New Window",grey_img);
        //fraps++;
        //if (fraps%10==0)
        {
            //memcpy(data_background, grey_data, sizeof(data_background));
            //imshow("background",background);
        }

        if (waitKey(5) >= 0) break;
    }
    //Camera.Close();
    waitKey(0);
}

    