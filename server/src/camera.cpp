#include "config.h"
#include <ctime>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include "camera.hpp"
#include "video_s.hpp"
#include "v_analyse.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/xfeatures2d.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/video/tracking.hpp"
//#include "opencv2/cvstd.hpp"

#define DEBUG_CAM printf

ledClass led_object;
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
    if (type_p == CAM_AUTO_DRIVE){
        post_process_img = Mat(height, width, CV_8UC1, post_process_data);
        for(uint32_t i = 0; i< height; i++)
            data_mat[i] = &post_process_data[i*width];
    }
    type = type_p;
    DEBUG_CAM("CAM: Camera init ");
    switch(type)
    {
        case CAM_STREAM:
        DEBUG_CAM("CAM_STREAM\n");
        break;
        case CAM_AUTO_DRIVE:
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

Ptr<ORB> detector=ORB::create();
std::vector<KeyPoint> keypoints_1, keypoints_2;
Mat descriptors_1, descriptors_2;
//FlannBasedMatcher matcher;
Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("BruteForce-Hamming");
std::vector< DMatch > matches;
std::vector< DMatch > good_matches;
Mat cmpImg, outputImg;
double max_dist = 0; double min_dist = 100;
static uint32_t counter;

void featureDetection(Mat img_1, vector<Point2f>& points1)	{   
    //uses FAST as of now, modify parameters as necessary
  vector<KeyPoint> keypoints_1;
  int fast_threshold = 20;
  bool nonmaxSuppression = true;
  //printf("Keypoint before size: %d\n", keypoints_1.size());
  detector->detectAndCompute(img_1, Mat(), keypoints_1, descriptors_1);
  //FAST(img_1, keypoints_1, fast_threshold, nonmaxSuppression);
  printf("Keypoint after size: %d\n", keypoints_1.size());
  KeyPoint::convert(keypoints_1, points1, vector<int>());
}

void featureTracking(Mat img_1, Mat img_2, vector<Point2f>& points1, vector<Point2f>& points2, vector<uchar>& status)	{ 

//this function automatically gets rid of points for which tracking fails

  vector<float> err;					
  Size winSize=Size(21,21);																								
  TermCriteria termcrit=TermCriteria(TermCriteria::COUNT+TermCriteria::EPS, 30, 0.01);

  calcOpticalFlowPyrLK(img_1, img_2, points1, points2, status, err, winSize, 3, termcrit, 0, 0.001);

  //getting rid of points for which the KLT tracking failed or those who have gone outside the frame
  
  int indexCorrection = 0;
  for( int i=0; i<status.size(); i++)
     {  Point2f pt = points2.at(i- indexCorrection);
     	if ((status.at(i) == 0)||(pt.x<0)||(pt.y<0))	{
     		  if((pt.x<0)||(pt.y<0))	{
     		  	status.at(i) = 0;
     		  }
     		  points1.erase (points1.begin() + (i - indexCorrection));
     		  points2.erase (points2.begin() + (i - indexCorrection));
     		  indexCorrection++;
     	}
        else
        {
                line(img_1,points2[i], points1[i], Scalar(0, 0, 255), 2);
                //circle(img_1, points2[i], 5, Scalar(0, 0, 255), -1);
        }

     }
     
}

double x =0, y=0, z = 0;
double x_prev, y_prev, z_prev;

void piCamera::process(void)
{
    
    DEBUG_CAM("CAM: Star cam process. Cam type == %d\n", type);
    vector<uchar> status;
    Mat E, R, t, mask;
    Mat prevImage;
    Mat currImage;
    vector<Point2f> prevFeatures;
    vector<Point2f> currFeatures;
    double focal = 1783.32274;
    cv::Point2d pp(188.30225, 85.66158);
    vector<Point2f> points1, points2;
    Mat R_f, t_f;
    Mat traj = Mat::zeros(600, 600, CV_8UC3);
    char text[100];
    cv::Point textOrg(10, 50);
    int fontFace = FONT_HERSHEY_PLAIN;
    double fontScale = 1;
    double scale = 1.00;
    int thickness = 1;
    int x = 300;
    int y = 100;
    while(1)
    {
        if (state != CAM_START)
        {
            sleep(1);
            DEBUG_CAM("CAM: Cam not start, sleep\n");
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
        if (type == CAM_STREAM)
        {
            sem_post (&sem_img_ready);
            //imshow("Grey_img",grey_img);
        }      

        if (type == CAM_AUTO_DRIVE)
        {
            treshold(grey_data, post_process_data, img_size/3, 200);
            this->find_object();
            /////////////////////////////////////////////
            //detector->detectAndCompute(grey_img, Mat(), keypointsD, descriptor);
            if (counter == 0)
            {
                grey_img.copyTo(cmpImg);
                //detector->detectAndCompute( cmpImg, Mat(), keypoints_1, descriptors_1 );
                featureDetection(cmpImg, points1);
                counter++;
                continue;
            }
            else if(counter == 1)
            {
                cout << "E = "<< endl << " "  << E << endl << endl;
                cout << "R = "<< endl << " "  << R << endl << endl;
                cout << "t = "<< endl << " "  << t << endl << endl;
                featureTracking(cmpImg,grey_img,points1,points2, status);
                E = findEssentialMat(points2, points1, focal, pp, RANSAC, 0.999, 1.0, mask);
                recoverPose(E, points2, points1, R, t, focal, pp, mask);
                cout << "E = "<< endl << " "  << E << endl << endl;
                cout << "R = "<< endl << " "  << R << endl << endl;
                cout << "t = "<< endl << " "  << t << endl << endl;
                sleep(5);
                cmpImg = grey_img;
                R_f = R.clone();
                t_f = t.clone();
                counter++;
                continue;
            }
            counter++; 
            
            featureTracking(cmpImg,grey_img,points1,points2, status);
            E = findEssentialMat(points2, points1, focal, pp, RANSAC, 0.999, 0.1, mask);
            recoverPose(E, points2, points1, R, t, focal, pp, mask);
            
            t = t / sqrt(t.at<double>(1, 0) * t.at<double>(1, 0) + t.at<double>(2, 0) * t.at<double>(2, 0) +
                 t.at<double>(0, 0) * t.at<double>(0, 0));

            imshow( "img1", cmpImg );
            imshow( "img2", grey_img );
            cout << "t = "<< endl << " "  << t << endl << endl;
            waitKey(2000);

            Mat prevPts(2,points2.size(), CV_64F), currPts(2,points1.size(), CV_64F);
            for(int i=0;i<points2.size();i++)	{   //this (x,y) combination makes sense as observed from the source code of triangulatePoints on GitHub
  		        prevPts.at<double>(0,i) = points2.at(i).x;
  		        prevPts.at<double>(1,i) = points2.at(i).y;

  		        currPts.at<double>(0,i) = points1.at(i).x;
  		        currPts.at<double>(1,i) = points1.at(i).y;
            }

            //scale = getAbsoluteScale(1000, 0, t.at<double>(2));
            scale = 5;
            //if ((scale>0.1)&&(t.at<double>(2) > t.at<double>(0)) && (t.at<double>(2) > t.at<double>(1))) {

                t_f = t_f + scale*(R_f*t);
                R_f = R*R_f;

            // }
  	
            // else {
            //     cout << "scale below 0.1, or incorrect translation" << endl;
            // }

            if (points2.size() < 150)	{
                cout << "Number of tracked features reduced to " << points2.size() << endl;
                cout << "trigerring redection" << endl;
 		        featureDetection(cmpImg, points1);
                featureTracking(cmpImg,grey_img,points1,points2, status);

 	        }
            
            cmpImg = grey_img.clone();
            points2 = points1;
            printf ("t.at<double>(0): %f,(2): %f", t.at<double>(0), t.at<double>(2));
            x += int(t.at<double>(0)*scale);
            y += int(t.at<double>(2)*scale);
            circle(traj, Point(x, y) ,1, CV_RGB(255,0,0), 2);

            rectangle( traj, Point(10, 30), Point(550, 50), CV_RGB(0,0,0), CV_FILLED);
            sprintf(text, "Coordinates: x = %d y = %d z = %d",x, y, z);// t_f.at<double>(0), t_f.at<double>(1), t_f.at<double>(2));
            putText(traj, text, textOrg, fontFace, fontScale, Scalar::all(255), thickness, 8);
            
            imshow( "Trajectory", traj );
            
            /*
            detector->detectAndCompute( grey_img, Mat(), keypoints_2, descriptors_2 ); 
            matcher->match( descriptors_1, descriptors_2, matches );
            //drawKeypoints(color_img, keypointsD, color_img);
            good_matches.clear();
            min_dist = 100;
            for( int i = 0; i < descriptors_1.rows; i++ )
            { double dist = matches[i].distance;
                if( dist < min_dist ) min_dist = dist;
                if( dist > max_dist ) max_dist = dist;
            }
            for( int i = 0; i < descriptors_1.rows; i++ )
            { if( matches[i].distance <= max(2*min_dist, 0.02) )
                { good_matches.push_back( matches[i]); }
            }

            drawMatches( cmpImg, keypoints_1, grey_img, keypoints_2,
               good_matches, outputImg, Scalar::all(-1), Scalar::all(-1),
               vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );
               */
            //imshow("New Window",outputImg);
            waitKey(0);
        } 

    }    
}

    