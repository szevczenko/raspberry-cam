#ifndef _CAMERA_HPP_
#define _CAMERA_HPP_
#include <raspicam/raspicam.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "config.h"
using namespace std;
using namespace cv;
using namespace raspicam;

class piCamera
{
    public:
    piCamera(void);
    int state, type;
    int init(int, int, unsigned char*, int);
    void deinit(void);
    int startCam(void);
    int stopCam(void);
    RaspiCam Camera; //Cmaera object
    #if CONFIG_USE_WINDOW
    Mat color_img;
    Mat grey_img, post_process_img;
    #endif
    unsigned char * color_data, *grey_data, *post_process_data;
    int cam_width, cam_height, img_size;
    unsigned char *data_mat[CAM_HEIGHT]; //data_mat[row][col]
    unsigned char mem_pix[CAM_HEIGHT][CAM_WIDTH];
    void process(void);
    void find_object(void);
};

typedef enum
{
    CAM_NO_INIT = 0,
    CAM_INIT,
    CAM_DEINIT,
    CAM_STOP,
    CAM_START

}camState;

typedef enum
{
    CAM_NO_PROCESS,
    CAM_STREAM,
    CAM_LOCALIZATION,
    CAM_AUTO_DRIVE
    
}enCamDo;

#endif