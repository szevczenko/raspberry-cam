#ifndef ROPOT_PARAM_H
#define ROPOT_PARAM_H

#include <semaphore.h>
#include <pthread.h>

typedef struct
{
	double x;
	double y;
}position_t;

typedef struct 
{
	double x;
	double y;
	double R;
}circle_t;

typedef enum
{
	CONFIG_SET_MARKER_A,
	CONFIG_SET_MARKER_B,
	CONFIG_SET_MARKER_C,

}config_enum;

typedef struct
{
	position_t markerA;
	position_t markerB;
	position_t markerC;
	int is_actual_markerA;
	int is_actual_markerB;
	int is_actual_markerC;
}screen_position_t;

typedef struct
{
	position_t position_robot;
	position_t debug_point;
	position_t position_markerA;
	position_t position_markerB;
	position_t position_markerC;
	circle_t circleAB;
	circle_t circleBC;
	double alpha;
	double beta;
	double speed;
	double orientation;
}config_robot_t;

typedef struct
{
	position_t direction_versor;
	double rotation_speed;
	int rotatoin_flag;
	double motion_speed;
	int motion_flag;
	double time;
}robot_priv_t;

extern config_robot_t config_robot;
extern robot_priv_t config_robot_priv;
extern screen_position_t markers_screen;

void test_robot(void);

void rp_change_type_motion(int type);
void rp_find_markers(int count, double time);

void rp_init(void);
void start_find_process(pthread_attr_t	*attr);
void wait_to_end_rp_loc(void);

#endif