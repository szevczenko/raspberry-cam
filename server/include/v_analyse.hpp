#ifndef _V_ANALYSE_HPP
#define _V_ANALYSE_HPP

#include "config.h"
#include "stdint.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

#define DISTANCE_AV 10

class ledClass
{
    public:
    ledClass(void);
    double x_av;
    double y_av;
    int x_min, x_max, y_min, y_max;
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
    private:
    int cp; //liczba pikseli dla obliczen wewnetrznych
    int period;
    int state_idf;
    int nominator_x, nominator_y;
    int denominator;
    void delete_from_tab(int i);
}; 

int go(unsigned char *array ,ledClass *tab_ob, int x_max, int x, int y);

#endif