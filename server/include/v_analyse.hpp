#ifndef _V_ANALYSE_HPP
#define _V_ANALYSE_HPP

#include "config.h"
#include "stdint.h"

using namespace std;

class ledClass
{
    public:
    double x_av;
    double y_av;
    int count_pixel;
    void add_pixel(int x, int y, int w);
    void count_average(void);
    private: 
    int nominator_x, nominator_y;
    int denominator;
}; 

int go(uint8_t array[][CAM_WIDTH],ledClass *tab_ob, uint8_t x, uint8_t y);

#endif