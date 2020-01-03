#if 0
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

int featureDetection(Mat img_1, vector<Point2f>& points1)	{   
    //uses FAST as of now, modify parameters as necessary
  vector<KeyPoint> keypoints_1;
  int fast_threshold = 20;
  bool nonmaxSuppression = true;
  //printf("Keypoint before size: %d\n", keypoints_1.size());
  detector->detectAndCompute(img_1, Mat(), keypoints_1, descriptors_1);
  //FAST(img_1, keypoints_1, fast_threshold, nonmaxSuppression);
  //printf("Keypoint after size: %d\n", keypoints_1.size());
  if (keypoints_1.size() < 5) return 0;
  KeyPoint::convert(keypoints_1, points1, vector<int>());
  return keypoints_1.size();
}

int featureTracking(Mat img_1, Mat img_2, vector<Point2f>& points1, vector<Point2f>& points2, vector<uchar>& status)	
{ 
//this function automatically gets rid of points for which tracking fails

  vector<float> err;					
  Size winSize=Size(21,21);																								
  TermCriteria termcrit=TermCriteria(TermCriteria::COUNT+TermCriteria::EPS, 30, 0.01);

  calcOpticalFlowPyrLK(img_1, img_2, points1, points2, status, err, winSize, 3, termcrit, 0, 0.001);

  //getting rid of points for which the KLT tracking failed or those who have gone outside the frame
  
  int indexCorrection = 0;
  for( int i=0; i<status.size(); i++)
    {  
        Point2f pt = points2.at(i- indexCorrection);
     	if ((status.at(i) == 0)||(pt.x<0)||(pt.y<0))	
        {
     		if((pt.x<0)||(pt.y<0))	
            {
     		    status.at(i) = 0;
     		}
     		points1.erase (points1.begin() + (i - indexCorrection));
     		points2.erase (points2.begin() + (i - indexCorrection));
     		indexCorrection++;
     	}
        // else
        // {
        //         line(img_1,points2[i], points1[i], Scalar(0, 0, 255), 2);
        //         //circle(img_1, points2[i], 5, Scalar(0, 0, 255), -1);
        // }

    }
     return points1.size();
}

//double x =0, y=0, z = 0;
double x_prev, y_prev, z_prev;
static uint32_t frame_cnt;

int x = 300;
int y = 100;
int z = 0;
static double focal = 1783.32274;
static cv::Point2d pp(188.30225, 85.66158);
static Mat traj;

void piCamera::find_motion_vector(void)
{
    static vector<uchar> status;
    static Mat E, R, t, mask;
    static Mat prevImage;
    static Mat currImage;
    static vector<Point2f> prevFeatures;
    static vector<Point2f> currFeatures;
    static vector<Point2f> points1, points2;
    static char text[100];
    static cv::Point textOrg(10, 50);
    static int fontFace = FONT_HERSHEY_PLAIN;
    static double fontScale = 1;
    static double scale = 1.00;
    static int thickness = 1;
    if (counter == 0)
    {
        grey_img.copyTo(cmpImg);
        if (featureDetection(cmpImg, points1) == 0)
        {
            DEBUG_CAM("CAM: Feature not found\n");
            return;
        } 
        printf("CAM: points1 cnt %d", points1.size());
        counter++;
        return;
    }
    else if(counter == 1)
    {
        if (featureTracking(cmpImg,grey_img,points1,points2, status) < 10)
        {
            counter = 0;
            return;
        }
        E = findEssentialMat(points2, points1, focal, pp, RANSAC, 0.999, 1.0, mask);
        recoverPose(E, points2, points1, R, t, focal, pp, mask);
        // cout << "E = "<< endl << " "  << E << endl << endl;
        // cout << "R = "<< endl << " "  << R << endl << endl;
        // cout << "t = "<< endl << " "  << t << endl << endl;
        cmpImg = grey_img;
        counter++;
        return;
    }
    counter++; 
    DEBUG_CAM("CAM: Feature tracking\n");
    if (featureTracking(cmpImg,grey_img,points1,points2, status) < 10) 
    {
        counter = 0;
        return;
    }
    DEBUG_CAM("CAM: Find essential\n");
    E = findEssentialMat(points2, points1, focal, pp, RANSAC, 0.999, 0.1, mask);
    DEBUG_CAM("CAM: recoverPose\n");
    recoverPose(E, points2, points1, R, t, focal, pp, mask);
            
    // t = t / sqrt(t.at<double>(1, 0) * t.at<double>(1, 0) + t.at<double>(2, 0) * t.at<double>(2, 0) +
    //              t.at<double>(0, 0) * t.at<double>(0, 0));

    //imshow( "img1", cmpImg );
    cout << "t = "<< endl << " "  << t << endl << endl;

    //scale = getAbsoluteScale(1000, 0, t.at<double>(2));
    scale = 1;

    if (points2.size() < 150)	
    {
        cout << "Number of tracked features reduced to " << points2.size() << endl;
        cout << "trigerring redection" << endl;
 		if (featureDetection(cmpImg, points1) == 0) return;
        if (featureTracking(cmpImg,grey_img,points1,points2, status) < 10)
        {
            counter = 0;
            return;
        }

 	}
            
    cmpImg = grey_img.clone();
    points2 = points1;
    // printf ("t.at<double>(0): %f,(2): %f", t.at<double>(0), t.at<double>(2));
    x += int(t.at<double>(0)*scale);
    y += int(t.at<double>(2)*scale);
    circle(traj, Point(x, y) ,1, CV_RGB(255,0,0), 2);

    rectangle( traj, Point(10, 30), Point(550, 50), CV_RGB(0,0,0), CV_FILLED);
    sprintf(text, "Coordinates: x = %f y = %f z = %f",x, y, z);// t_f.at<double>(0), t_f.at<double>(1), t_f.at<double>(2));
    putText(traj, text, textOrg, fontFace, fontScale, Scalar::all(255), thickness, 8);
    
}
#endif