#include "v_analyse.hpp"
#include "stdio.h"
#include "string.h"
#include "time.h"
#include "cmd.hpp"
#include "robot_param.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <chrono>


using namespace cv;

#define DEBUG_VA printf

static ledClass led_tab[30];
static int count_object;
int led_filling_tab[] = {25, 75, 50}; // A = 25, B = 75, C=50


ledClass::ledClass(void)
{
    n_led = -1;
}

void ledClass::accept(void)
{
    //DEBUG_VA("VA: accept %d\n\r", count_object);
    if (count_object < 30)
    {
        n_led = 0;
        memcpy(&led_tab[count_object], this, sizeof(ledClass));
        count_object++;
    }
}

int ledClass::check(void)
{
    int temp_led[30];
    double min_displacement, displacement;
    int temp_min_led_id = 0;
    int temp_led_cnt = 0;
    for(int i = 0; i < count_object; i++)
    {
         if (x_av >= led_tab[i].x_av - DISTANCE_AV_X \
            && x_av <= led_tab[i].x_av + DISTANCE_AV_X \
            && y_av >= led_tab[i].y_av - DISTANCE_AV_Y \
            && y_av <= led_tab[i].y_av + DISTANCE_AV_Y)
        {
            //DEBUG_VA("VA: ZNALEZIONE PODOBIENSTWO\n\r");
            temp_led[temp_led_cnt] = i;
            if (temp_led_cnt == 0)
            {
                temp_min_led_id = i;
                min_displacement = (x_av - led_tab[i].x_av)*(x_av - led_tab[i].x_av) + (y_av - led_tab[i].y_av)*(y_av - led_tab[i].y_av);
            }
            else
            {
                displacement = (x_av - led_tab[i].x_av)*(x_av - led_tab[i].x_av) + (y_av - led_tab[i].y_av)*(y_av - led_tab[i].y_av);
                if (min_displacement > displacement)
                {
                    min_displacement = displacement;
                    temp_min_led_id = i;
                }
            }
            temp_led_cnt++;
        }
    }
    if (temp_led_cnt == 0)
    {
      accept();
    }
    else if(temp_led_cnt == 1)
    {
        if (led_tab[temp_min_led_id].state_idf == 1)
        {
            if (led_tab[temp_min_led_id].y_av < y_av)
            return 0;
        }
        //DEBUG_VA("ANALYSE: Object %d found in table\n", i);
        led_tab[temp_min_led_id].state_idf = 1;
        led_tab[temp_min_led_id].x_displace = (int)(x_av - led_tab[temp_min_led_id].x_av);
        led_tab[temp_min_led_id].y_displace = (int)(y_av - led_tab[temp_min_led_id].y_av);
        led_tab[temp_min_led_id].x_av = x_av;
        led_tab[temp_min_led_id].y_av = y_av;
        led_tab[temp_min_led_id].count_pixel = count_pixel;
    }
    else
    {
        uint32_t counter_live = 0;
        for (int k = 0; k < temp_led_cnt; k++)
        {
            if (counter_live < led_tab[temp_led[k]].counter_fraps)
            {
                counter_live = led_tab[temp_led[k]].counter_fraps;
                temp_min_led_id = temp_led[k];
            }

        }

        led_tab[temp_min_led_id].state_idf = 1;
        led_tab[temp_min_led_id].x_displace = (int)(x_av - led_tab[temp_min_led_id].x_av);
        led_tab[temp_min_led_id].y_displace = (int)(y_av - led_tab[temp_min_led_id].y_av);
        led_tab[temp_min_led_id].x_av = x_av;
        led_tab[temp_min_led_id].y_av = y_av;
        led_tab[temp_min_led_id].count_pixel = count_pixel;
        //DEBUG_VA("VA: delete cnt %d, wzorzec %d, min %f\n\r Delete: ", temp_led_cnt, temp_min_led_id, min_displacement);
        for (int k = 0; k < temp_led_cnt; k++)
        {
            if (temp_min_led_id != temp_led[k])
            {
                //DEBUG_VA("%d, ", temp_led[k]);
                delete_from_tab(temp_led[k]);
            }

        }
    }

    return 0;
}
static Point pt;

void ledClass::delete_from_tab(int i)
{
    count_object--;
    if (i != count_object)
        memcpy(&led_tab[i], &led_tab[count_object], sizeof(ledClass));
}

// void ledClass::pixel_displacement_non_active_led(void)
// {

// }
std::chrono::time_point<std::chrono::high_resolution_clock> now_time, previos_time;

void ledClass::post_process(Mat *img)
{
    int dis_number = 0;
    for (int i = 0; i < count_object; i++)
    {
        if (led_tab[i].state_idf == 1)
        {
            led_tab[i].state_idf = 0;
            led_tab[i].fraps_active++;
            if (dis_number == 0)
            {
               dis_number = i + 1;
            }
        }
        else
        {
            led_tab[i].non_active_flag = 1;
            led_tab[i].fraps_non_active++;
        }
        if (led_tab[i].counter_fraps % 60 == 0 && led_tab[i].counter_fraps != 0)
        {
            led_tab[i].filling = (double)led_tab[i].fraps_active / (double)(led_tab[i].fraps_active + led_tab[i].fraps_non_active) * 100;
            if (led_tab[i].n_led == 0)
            {
                for (uint8_t j = 0; j < sizeof(led_filling_tab)/sizeof(int); j++)
                {
                    if (led_tab[i].filling <= led_filling_tab[j] + 10 && led_tab[i].filling >= led_filling_tab[j] - 10)
                        led_tab[i].n_led = j + 1;
                }
            }
            else
            {
                if (led_tab[i].filling <= led_filling_tab[led_tab[i].n_led - 1] + 10 && led_tab[i].filling >= led_filling_tab[led_tab[i].n_led - 1] - 10)
                {

                }
                else
                {
                  led_tab[i].n_led = 0;
                }
            }
            if (led_tab[i].n_led == 0)
            {
                //DEBUG_VA("ANALYSE: DELETED led %d, filling: %f, nb: %d, (x,y): %d, %d\n",i, led_tab[i].filling, led_tab[i].n_led, led_tab[i].x_displace, led_tab[i].y_displace);
                delete_from_tab(i);
                return;
            }
            else
            {
                led_tab[i].fraps_active = 0;
                led_tab[i].fraps_non_active = 0;
                //led_tab[i].counter_fraps = 0;
            }
        }
        led_tab[i].counter_fraps++;
        //DEBUG_VA("VA: id: %d, x=%f, y=%f\n\r", i, led_tab[i].x_av,led_tab[i].y_av);
    }
    //if (dis_number == 0) return;
    int led_cnt = 0;
    for (int i = 0; i < count_object; i++)
    {
        switch (led_tab[i].n_led)
        {
            case 1:
            if (markers_screen.is_actual_markerA || led_tab[i].counter_fraps < 151)
                break;
            markers_screen.markerA.x = led_tab[i].x_av;
            markers_screen.markerA.y = led_tab[i].y_av;
            markers_screen.is_actual_markerA = 1;
            led_cnt++;
            break;

            case 2:
            if (markers_screen.is_actual_markerB || led_tab[i].counter_fraps < 151)
                break;
            markers_screen.markerB.x = led_tab[i].x_av;
            markers_screen.markerB.y = led_tab[i].y_av;
            markers_screen.is_actual_markerB = 1;
            led_cnt++;
            break;

            case 3:
            if (markers_screen.is_actual_markerC || led_tab[i].counter_fraps < 151)
                break;
            markers_screen.markerC.x = led_tab[i].x_av;
            markers_screen.markerC.y = led_tab[i].y_av;
            markers_screen.is_actual_markerC = 1;
            led_cnt++;
            break;
        }
        if (led_tab[i].non_active_flag == 1)
        {
            led_tab[i].non_active_flag = 0;
            switch (cmd_go_get_state())
            {
                case GO_STR:
                case GO_BACK:

                break;

                case GO_LEFT:
                case GO_RIGHT:
                case GO_STOP:
                    led_tab[i].x_av += (double)led_tab[dis_number - 1].x_displace;
                break;
                default:
                break;
            }
        }
    }
    now_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_time = now_time - previos_time;
    rp_find_markers(led_cnt, duration_time.count());
    previos_time = now_time;
}

void ledClass::draw_object(Mat *img)
{
    for (int i = 0; i < count_object; i++)
    {
        if (led_tab[i].n_led != 0)
        {
            pt.x = led_tab[i].x_av;
            pt.y = led_tab[i].y_av;
            switch (led_tab[i].n_led)
            {
                case 1:
                circle(*img, pt, DISTANCE_AV_X, Scalar(0,0,255));
                break;
                case 2:
                circle(*img, pt, DISTANCE_AV_X, Scalar(255,0,0));
                break;
                case 3:
                circle(*img, pt, DISTANCE_AV_X, Scalar(0,255,0));
                break;
                default:
                circle(*img, pt, DISTANCE_AV_X, Scalar(255,255,255));
                break;
            }
        }
        else
        {
            pt.x = led_tab[i].x_av;
            pt.y = led_tab[i].y_av;
            circle(*img, pt, DISTANCE_AV_X, Scalar(255,255,255));
        }
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
