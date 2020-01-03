#ifndef _LOCATION_HPP_
#define _LOCATION_HPP_
#include "config.h"
#include <stdio.h>
#include <stdint.h>

#define COUNT_LED 3

typedef enum
{
    LED_1 = 1,
    LED_2,
    LED_3
}led_number_t;

typedef struct 
{
    uint8_t led_number;
    int x, y, z; //global
    double filling;
}led_struct;


#endif