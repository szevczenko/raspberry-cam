#include "v_analyse.hpp"
#include "stdio.h"

#define DEBUG_VA printf

void ledClass::add_pixel(int x, int y, int w) // w - waga
{
    nominator_x += w*x;
    nominator_y += w*y;
    denominator += w;
    count_pixel++;
}

void ledClass::count_average(void)
{
    x_av = (double)nominator_x / (double)denominator;
    y_av = (double)nominator_y / (double)denominator;
    nominator_x = 0;
    nominator_y = 0;
    denominator = 0;
    DEBUG_VA ("sr: x=%f, y=%f, count=%d\n", x_av, y_av, count_pixel);
}

int go(uint8_t array[][CAM_WIDTH],ledClass *tab_ob, uint8_t x, uint8_t y)
{
    if (array[x][y] > 0)
    {
        DEBUG_VA ("x: %d, y: %d\n",x,y);
        tab_ob->add_pixel(x, y, array[x][y]);
        array[x][y] = 0;
        if (x < CAM_WIDTH - 1)
            go(array, tab_ob, x+1, y);
        if (y < CAM_HEIGHT - 1)
            go(array, tab_ob, x, y+1);
        if (x > 0)
            go(array, tab_ob, x-1, y);
        if (y > 0)
            go(array, tab_ob, x, y-1);
        return 1;
    }
    else return 0;
}