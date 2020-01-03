#ifndef _V_ANALYSE_HPP
#define _V_ANALYSE_HPP

#include "config.h"
#include "stdint.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <semaphore.h>
#include <pthread.h>

using namespace std;
using namespace cv;

#define DISTANCE_AV_X 17
#define DISTANCE_AV_Y 75

class ledClass
{
    public:
    ledClass(void);
    double x_av;
    double y_av;
    int x_min, x_max, y_min, y_max;
    int x_displace, y_displace;
    int n_led;
    int count_pixel;
    double filling; //wypelnienie
    int fraps_active;
    int fraps_non_active;
    int counter_fraps;
    void add_pixel(int x, int y, int w);
    void count_average(void);
    void accept(void);
    int check(void);
    void post_process(Mat *img);
    void draw_object(Mat *img);
    private:
    int cp; //liczba pikseli dla obliczen wewnetrznych
    int period;
    int state_idf;
    int non_active_flag;
    int nominator_x, nominator_y;
    int denominator;
    void delete_from_tab(int i);
};

int go(unsigned char *array ,ledClass *tab_ob, int x_max, int x, int y);

#endif
