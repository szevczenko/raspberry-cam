#include "stdint.h"
#include "string.h"
#include "math.h"
#include "robot_param.hpp"

config_robot_t config_robot;

void test_robot(void)
{
    config_robot.position_markerA.x = 0;
    config_robot.position_markerA.y = 100;
    config_robot.position_markerB.x = 0;
    config_robot.position_markerB.y = 0;
    config_robot.position_markerC.x = 100;
    config_robot.position_markerC.y = 0;
    config_robot.alpha = 1.2;
    config_robot.beta = 0.4;
    config_robot.position_robot.x = 300;
    config_robot.position_robot.y = 50;
    config_robot.circleAB.x = 25;
    config_robot.circleAB.y = 25;
    config_robot.circleBC.x = 25;
    config_robot.circleBC.y = 50;
    config_robot.circleAB.R = 20;
    config_robot.circleBC.R = 25;
}

int robot_param_parse_data(uint8_t * data, int len)
{
    if (len != sizeof(config_robot_t)) 
    {
        printf("ROBOT: len != sizeof(config_robot_t) %d != %d",len, sizeof(config_robot_t));
        return 0;
    }
    memcpy(&config_robot, data, sizeof(config_robot_t));
    return 1;
}

#define OFFSET 100
#define C_OFFSET 75

int config_visualization(cv::Mat * image, config_robot_t * config)
{
    char text[32];
    sprintf(text, "robot x: %d, y: %d", (int)config->position_robot.x, (int)config->position_robot.y);
    circle(*image, Point(config->position_markerA.x + OFFSET, config->position_markerA.y + OFFSET), 3 ,Scalar(255, 255, 255), CV_FILLED);
    putText(*image, "A", Point(config->position_markerA.x + C_OFFSET, config->position_markerA.y + OFFSET), CV_FONT_ITALIC, 0.5, Scalar::all(255));
    circle(*image, Point(config->position_markerB.x + OFFSET, config->position_markerB.y + OFFSET), 3 ,Scalar(255, 255, 255), CV_FILLED);
    putText(*image, "B", Point(config->position_markerB.x + C_OFFSET, config->position_markerB.y + OFFSET), CV_FONT_ITALIC, 0.5, Scalar::all(255));
    circle(*image, Point(config->position_markerC.x + OFFSET, config->position_markerC.y + OFFSET), 3 ,Scalar(255, 255, 255), CV_FILLED);
    putText(*image, "C", Point(config->position_markerC.x + C_OFFSET, config->position_markerC.y + OFFSET), CV_FONT_ITALIC, 0.5, Scalar::all(255));
    circle(*image, Point(config->position_robot.x + OFFSET, config->position_robot.y + OFFSET), 3 ,Scalar(255, 0, 255), CV_FILLED);
    arrowedLine(*image, Point(config->position_robot.x + OFFSET, config->position_robot.y + OFFSET), Point(config->position_robot.x + OFFSET + 15*cos(config->orientation), config->position_robot.y + OFFSET + 15*sin(config->orientation)),Scalar(255, 255, 255), 1);
    //line(*image, Point(config->circleAB.x + OFFSET, config->circleAB.y + OFFSET), Point(config->circleBC.x + OFFSET, config->circleBC.y + OFFSET),Scalar(255, 255, 255), 1);
    putText(*image, text, Point(config->position_robot.x + 10, config->position_robot.y + 35), CV_FONT_ITALIC, 0.35, Scalar::all(255));
    circle(*image, Point(config->circleAB.x + OFFSET, config->circleAB.y + OFFSET), config->circleAB.R ,Scalar(255, 0, 255));
    circle(*image, Point(config->circleBC.x + OFFSET, config->circleBC.y + OFFSET), config->circleBC.R ,Scalar(255, 0, 255));

    //putText(*image, "D", Point(config->debug_point.x + C_OFFSET, config->debug_point.y + OFFSET), CV_FONT_ITALIC, 0.5, Scalar::all(255));
    //circle(*image, Point(config->debug_point.x + OFFSET, config->debug_point.y + OFFSET), 3 ,Scalar(255, 255, 255), CV_FILLED);
    return 1;
}  