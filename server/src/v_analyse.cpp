#include "v_analyse.hpp"
#include "stdio.h"
#include "string.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

#define DEBUG_VA printf

static ledClass led_tab[10];
static int count_object;
int led_filling_tab[] = {25, 50, 75};

ledClass::ledClass(void)
{
    n_led = -1;
}

void ledClass::accept(void)
{
    if (count_object < 10)
    {
        n_led = 0;
        memcpy(&led_tab[count_object], this, sizeof(ledClass));
        count_object++;
    }
}

int ledClass::check(void)
{
    for(int i = 0; i < count_object; i++)
    {
        if (x_av >= led_tab[i].x_min && x_av <= led_tab[i].x_max &&
            y_av >= led_tab[i].y_min && y_av <= led_tab[i].y_max)
            {
                //DEBUG_VA("ANALYSE: Object %d found in table\n", i);
                led_tab[i].state_idf = 1;
                led_tab[i].x_av = x_av;
                led_tab[i].y_av = y_av;
                led_tab[i].count_pixel = count_pixel;
                led_tab[i].x_min = x_av - DISTANCE_AV;
                led_tab[i].x_max = x_av + DISTANCE_AV;
                led_tab[i].y_min = y_av - DISTANCE_AV;
                led_tab[i].y_max = y_av + DISTANCE_AV;
                return 1;
            }
    }
    accept();
    return 0;
}
static Point pt;

void ledClass::delete_from_tab(int i)
{
    count_object--;
    if (i != count_object)
        memcpy(&led_tab[i], &led_tab[count_object], sizeof(ledClass));
}

void ledClass::post_process(Mat *img)
{
    for (int i = 0; i < count_object; i++)
    {
        if (led_tab[i].state_idf == 1)
        {
            led_tab[i].state_idf = 0;
            led_tab[i].fraps_active++;
        }
        else
        {
            led_tab[i].fraps_non_active++;
        }
        if (led_tab[i].counter_fraps >= 30)
        {
            led_tab[i].filling = (double)led_tab[i].fraps_active / (double)(led_tab[i].fraps_active + led_tab[i].fraps_non_active) * 100;
            led_tab[i].n_led = 0;
            for (uint8_t j = 0; j < sizeof(led_filling_tab)/sizeof(int); j++)
            {
                if (led_tab[i].filling <= led_filling_tab[j] + 10 && led_tab[i].filling >= led_filling_tab[j] - 10)
                    led_tab[i].n_led = j + 1;
            }
            if (led_tab[i].n_led == 0) 
            {
                delete_from_tab(i);
                return;
            }
            else
            {
                DEBUG_VA("ANALYSE: led %d, filling: %f, nb: %d\n",i, led_tab[i].filling, led_tab[i].n_led);
                led_tab[i].fraps_active = 0;
                led_tab[i].fraps_non_active = 0;
                led_tab[i].counter_fraps = 0;
            }
        }
        if (led_tab[i].n_led != 0)
        {
            pt.x = led_tab[i].x_av;
            pt.y = led_tab[i].y_av;
            switch (led_tab[i].n_led)
            {
                case 1:
                circle(*img, pt, DISTANCE_AV, Scalar(0,0,255));
                break;
                case 2:
                circle(*img, pt, DISTANCE_AV, Scalar(255,0,0));
                break;
                case 3:
                circle(*img, pt, DISTANCE_AV, Scalar(0,255,0));
                break;
                default:
                circle(*img, pt, DISTANCE_AV, Scalar(255,255,255));
                break;
            }
        }
        led_tab[i].counter_fraps++;
    }
}

void ledClass::add_pixel(int x, int y, int w) // w - waga
{
    nominator_x += w*x;
    nominator_y += w*y;
    denominator += w;
    cp++;
}

void ledClass::count_average(void)
{
    x_av = (double)nominator_x / (double)denominator;
    y_av = (double)nominator_y / (double)denominator;
    x_min = x_av - DISTANCE_AV;
    x_max = x_av + DISTANCE_AV;
    y_min = y_av - DISTANCE_AV;
    y_max = y_av + DISTANCE_AV;
    nominator_x = 0;
    nominator_y = 0;
    denominator = 0;
    count_pixel = cp;
    cp = 0;
    //DEBUG_VA ("sr: x=%f, y=%f, count=%d\n", x_av, y_av, count_pixel);
}

int go(unsigned char *array ,ledClass *tab_ob, int x_max, int x, int y)
{
    if (array[y*x_max + x] > 0)
    {
        tab_ob->add_pixel(x, y, array[y*x_max + x]);
        array[y*x_max + x] = 0;
        if (x < CAM_WIDTH - 1)
            go(array, tab_ob, x_max, x+1, y);
        if (y < CAM_HEIGHT - 1)
            go(array, tab_ob, x_max, x, y+1);
        if (x > 0)
            go(array, tab_ob, x_max, x-1, y);
        if (y > 0)
            go(array, tab_ob,x_max, x, y-1);
        return 1;
    }
    else return 0;
}