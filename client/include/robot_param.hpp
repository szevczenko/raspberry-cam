#ifndef ROPOT_PARAM_H
#define ROPOT_PARAM_H

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>

using namespace cv;

typedef struct
{
	double x;
	double y;
}position_t;

typedef struct 
{
	double x;
	double y;
	double R;
}circle_t;

typedef enum
{
	CONFIG_SET_MARKER_A,
	CONFIG_SET_MARKER_B,
	CONFIG_SET_MARKER_C,

}config_enum;

typedef struct
{
	position_t position_robot;
	position_t debug_point;
	position_t position_markerA;
	position_t position_markerB;
	position_t position_markerC;
	circle_t circleAB;
	circle_t circleBC;
	double alpha;
	double beta;
	double speed;
	double orientation;
}config_robot_t;

extern config_robot_t config_robot;


void test_robot(void);
int robot_param_parse_data(uint8_t * data, int len);
int config_visualization(cv::Mat * image, config_robot_t * config);

#endif