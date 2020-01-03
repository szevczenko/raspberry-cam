#include "string.h"
#include "robot_param.hpp"
#include "math.h"
#include <chrono>
#include "cmd.hpp"
#include "string.h"
#include "config.h"
#include "camera.hpp"

#define PI 3.14159265
config_robot_t config_robot, previos_config;
robot_priv_t config_robot_priv;
screen_position_t markers_screen, last_marker_screen;

void find_direction_vector(void);

void test_robot(void)
{
    config_robot.position_markerA.x = 0;
    config_robot.position_markerA.y = 50;
    config_robot.position_markerB.x = 0;
    config_robot.position_markerB.y = 0;
    config_robot.position_markerC.x = 50;
    config_robot.position_markerC.y = 0;
    config_robot.alpha = 1.2;
    config_robot.beta = 0.4;
    config_robot.position_robot.x = 200;
    config_robot.position_robot.y = 100;
    config_robot.circleAB.x = 25;
    config_robot.circleAB.y = 25;
    config_robot.circleBC.x = 25;
    config_robot.circleBC.y = 50;
    config_robot.circleAB.R = 20;
    config_robot.circleBC.R = 25;
    config_robot_priv.rotation_speed = 1;
    config_robot_priv.motion_speed = 1;
    find_direction_vector();
}

static int ev_cmd_state, ev_led_state;
static pthread_t find_thd;

static void * find_process(void * pv);

void rp_init(void)
{

}

void start_find_process(pthread_attr_t	*attr)
{
    pthread_create(&find_thd, attr, find_process, NULL);
}

void wait_to_end_rp_loc(void)
{
    pthread_join(find_thd, NULL);
}

int motion_state, last_motion_state;
int start_timer_state, start_gtimer_state;
std::chrono::time_point<std::chrono::high_resolution_clock> motion_time_start;
std::chrono::time_point<std::chrono::high_resolution_clock> motion_gtime_start;
std::chrono::time_point<std::chrono::high_resolution_clock> motion_time_change;
std::chrono::time_point<std::chrono::high_resolution_clock> motion_time_finish;
position_t position_start;
position_t g_position_start;
double orientation_start;
double g_orientation_start;

void find_direction_vector(void)
{
    config_robot_priv.direction_versor.x = cos(config_robot.orientation);
    config_robot_priv.direction_versor.y = sin(config_robot.orientation);
}

void robot_find_angle(void)
{
    double alpha;
    alpha = atan((config_robot.position_robot.y - config_robot.position_markerB.y)/ \
                        (config_robot.position_robot.x - config_robot.position_markerB.x));
    config_robot.orientation = - alpha - PI / 2;
    find_direction_vector();
}

void robot_increment_pos_speed(void)
{
    if (motion_state == GO_STR)
    {
        config_robot.position_robot.x += config_robot_priv.direction_versor.x * config_robot_priv.motion_speed * config_robot_priv.time;
        config_robot.position_robot.y += config_robot_priv.direction_versor.y * config_robot_priv.motion_speed * config_robot_priv.time;
    }
    if (motion_state == GO_BACK)
    {
        config_robot.position_robot.x -= config_robot_priv.direction_versor.x * config_robot_priv.motion_speed * config_robot_priv.time;
        config_robot.position_robot.y -= config_robot_priv.direction_versor.y * config_robot_priv.motion_speed * config_robot_priv.time;
    }

}

void robot_increment_orientation_speed(void)
{
    if (motion_state == GO_RIGHT)
        config_robot.orientation += config_robot_priv.time * config_robot_priv.rotation_speed;
    if (motion_state == GO_LEFT)
        config_robot.orientation -= config_robot_priv.time * config_robot_priv.rotation_speed;
    find_direction_vector();
}

static void rp_start_timer(void)
{
    start_timer_state = 1;
    motion_time_start = std::chrono::high_resolution_clock::now();
    position_start.x = config_robot.position_robot.x;
    position_start.y = config_robot.position_robot.y;
    orientation_start = config_robot.orientation;
}

static void rp_start_gtimer(void)
{
    start_gtimer_state = 1;
    motion_gtime_start = std::chrono::high_resolution_clock::now();
    g_position_start.x = config_robot.position_robot.x;
    g_position_start.y = config_robot.position_robot.y;
    g_orientation_start = config_robot.orientation;
}

static void rp_end_timer(double duration, int motion_type)
{
    if (start_timer_state == 0) return;
    if (motion_type == GO_STR || motion_type == GO_BACK)
    {
        printf("GO_STR: %f [s]\n", duration);
        //config_robot.speed = sqrt((config_robot.position_robot.x - position_start.x)*(config_robot.position_robot.x - position_start.x) + \
        //(config_robot.position_robot.y - position_start.y)*(config_robot.position_robot.y - position_start.y))/duration;
    }
    else
    {
        printf("ROTATE: %f [s]\n", duration);
        //config_robot_priv.rotation_speed = (orientation_start - config_robot.orientation)/duration;
    }
    start_timer_state = 0;
}

static void rp_end_gtimer(double duration, int motion_type)
{
    if (start_gtimer_state == 0) return;
    if (motion_type == GO_STR || motion_type == GO_BACK)
    {
        printf("G_GO_STR: %f [s]\n", duration);
        //config_robot.speed = sqrt((config_robot.position_robot.x - position_start.x)*(config_robot.position_robot.x - position_start.x) + \
        //(config_robot.position_robot.y - position_start.y)*(config_robot.position_robot.y - position_start.y))/duration;
    }
    else
    {
        printf("G_ROTATE: %f [s]\n", duration);
        //config_robot_priv.rotation_speed = (orientation_start - config_robot.orientation)/duration;
    }
    start_timer_state = 0;
}

static void rp_stop_timer(void)
{
    start_timer_state = 0;
}

static void rp_stop_gtimer(void)
{
    start_gtimer_state = 0;
}

int rp_timer_is_started(void)
{
    return start_timer_state;
}

int rp_gtimer_is_started(void)
{
    return start_gtimer_state;
}

void rp_change_type_motion(int type)
{
    last_motion_state = motion_state;
    motion_state = type;
    ev_cmd_state = 1;
    motion_time_change = std::chrono::high_resolution_clock::now();
    if (motion_state == GO_LEFT || motion_state == GO_RIGHT)
    {
        memcpy(&last_marker_screen, &markers_screen, sizeof(markers_screen));
    }
    if (type != GO_STOP)
    {
        rp_start_timer();
        rp_start_gtimer();
    }

}

static int markers_cnt;

void rp_find_markers(int count, double time)
{
    ev_led_state = 1;
    markers_cnt = count;
    config_robot_priv.time = time;
}

void find_screen_angle(void);
void find_circle_parameters(void);
extern piCamera * obCamPnt;
int counter_send;
void find_position(void);
void find_rotate_angle(double last_x, double previos_x);

void find_coordinate(void)
{
    find_screen_angle();
    find_circle_parameters();
    find_position();
    robot_find_angle();
}

static void * find_process(void * pv)
{
    while(1)
    {
        if(counter_send >= 15)
        {
            counter_send = 0;
            obCamPnt->send_position();
        }
        usleep(1000);
        if (ev_cmd_state)
        {
            ev_cmd_state = 0;
        }
        if (ev_led_state)
        {
            counter_send++;
            ev_led_state = 0;
            motion_time_finish = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed;
            switch (motion_state)
            {
                case GO_STR:
                case GO_BACK:
                    if (markers_cnt == 3)
                    {
                        elapsed = motion_time_finish - motion_time_start;
                        if (rp_timer_is_started() && elapsed.count() > 1 && motion_state != GO_STOP)
                        {
                            rp_end_timer(elapsed.count(), motion_state);
                            rp_start_timer();
                        }
                        find_coordinate();
                    }
                    else
                    {
                        robot_increment_pos_speed();
                    }
                break;

                case GO_LEFT:
                case GO_RIGHT:
                    if (markers_cnt != 0)
                    {
                        elapsed = motion_time_finish - motion_time_start;
                        if (rp_timer_is_started() && elapsed.count() > 1 && motion_state != GO_STOP)
                        {
                            rp_end_timer(elapsed.count(), motion_state);
                            rp_start_timer();
                        }
                    }
                    if (markers_cnt == 3)
                    {
                        find_coordinate();
                        break;
                    }
                    else if (markers_cnt == 0)
                    {
                        robot_increment_orientation_speed();
                        break;
                    }
                    if (markers_screen.is_actual_markerA && last_marker_screen.is_actual_markerA)
                    {
                        find_rotate_angle(last_marker_screen.markerA.x, markers_screen.markerA.x);
                        break;
                    }
                    if (markers_screen.is_actual_markerB && last_marker_screen.is_actual_markerB)
                    {
                        find_rotate_angle(last_marker_screen.markerB.x, markers_screen.markerB.x);
                        break;
                    }
                    if (markers_screen.is_actual_markerC && last_marker_screen.is_actual_markerC)
                    {
                        find_rotate_angle(last_marker_screen.markerC.x, markers_screen.markerC.x);
                        break;
                    }

                break;
                case GO_STOP:
                    if (markers_cnt == 3)
                    {
                        find_coordinate();
                    }
                    if (rp_timer_is_started() && last_motion_state != GO_STOP)
                    {
                        std::chrono::duration<double> time_motion = motion_time_change - motion_gtime_start;
                        if (markers_cnt == 3 && time_motion.count() > 1)
                        {
                            rp_end_gtimer(time_motion.count(), last_motion_state);
                        }
                    }
                break;
                default:
                break;
            }
            memcpy(&last_marker_screen, &markers_screen, sizeof(markers_screen));
            markers_screen.is_actual_markerA = 0;
            markers_screen.is_actual_markerB = 0;
            markers_screen.is_actual_markerC = 0;

        }
    }
}




#define Ym config_robot.circleAB.y
#define Xm config_robot.circleAB.x
#define Rm config_robot.circleAB.R
#define Yn config_robot.circleBC.y
#define Xn config_robot.circleBC.x
#define Rn config_robot.circleBC.R
#define Axs markers_screen.markerA.x
#define Bxs markers_screen.markerB.x
#define Cxs markers_screen.markerC.x
#define Ox config_robot.position_robot.x
#define Oy config_robot.position_robot.y

const double cx=CAM_WIDTH/2;
const double cy=CAM_HEIGHT/2;
const double fx=200.32274;
const double fy=2803.82717;

double _A, _As;
double _C, _Cs;
double _B, _Bs;

void find_screen_angle(void)
{
    _A = atan((Axs - cx)/fx);
    _B = atan((Bxs - cx)/fx);
    _C = atan((Cxs - cx)/fx);
    _As = _A*180/PI;
    _Bs = _B*180/PI;
    _Cs = _C*180/PI;
}

void find_rotate_angle(double last_x, double previos_x)
{
    _A = atan((last_x - cx)/fx);
    _B = atan((previos_x - cx)/fx);
    config_robot.orientation += -_B + _A;
}

void find_circle_parameters(void)
{
   Rn = -50/sin(_C-_B);
   Rm = -50/sin(_B-_A);
   if (Rn < 0)
   {
        printf("Rn zmienia znak\n");
        Rn = -Rn;
   }
   if (Rm < 0)
   {
        printf("Rm zmienia znak\n");
        Rm = -Rm;
   }
   Ym = 25;
   Xm = sqrt(Rm*Rm - 625);
   Yn = sqrt(Rn*Rn - 625);
   Xn = 25;
}

double l;
double cos_A, sin_A;
double Xp, Yp;

#define V1x Xm
#define V2x Xn
#define V3x 0
#define V4x Ox
#define V1y Ym
#define V2y Yn
#define V3y 0
#define V4y Oy

double u;

void find_position(void)
{
    l = sqrt((Xn-Xm)*(Xn-Xm) + (Yn-Ym)*(Yn-Ym));
    cos_A = (l*l + Rm*Rm - Rn*Rn)/(2*l*Rm);
    cos_A = cos(-acos(cos_A));
    sin_A = sqrt(1 - cos_A*cos_A);
    Xp = (Xn - Xm)*Rm/l;
    Yp = (Yn - Ym)*Rm/l;
    Ox = Xp*cos_A + Yp*sin_A + Xm;
    Oy = - Xp*sin_A + Yp*cos_A + Ym;
    config_robot.debug_point.x = Xp;
    config_robot.debug_point.y = Yp;
    if (Ox < 0)
    {
        Ox = 0;
    }
    if (Oy < 0)
    {
        Oy = 0;
    }
    // u = ((V3x - V1x)*(V2x - V1x) + (V3y - V1y)*(V2y - V1y))/((V1x - V2x)*(V1x - V2x) + (V1y - V2y)*(V1y - V2y));
    // V4x = V1x + (V1x - V2x)*u;
    // V4y = V1y + (V1y - V2y)*u;
}
