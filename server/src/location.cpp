// #include "location.hpp"

// ledLocationClass *led_table[COUNT_LED];
// void init_led_table(void)
// {

// }

// ledLocationClass::ledLocationClass(led_number_t led, double filling)
// {
//     if (led > LED_3) return;
//     led_table[led - 1]
// }

// int set_coordinates(int x, int y, int z)

// /*
//     parse_coordinate
//     1 byte - count led in msg
//     2 - ... bytes - data
// */

// int parse_coordinate(uint8_t *buff, int len) 
// {
//     int led_count = buff[0];
//     int len_msg = led_count * sizeof(led_struct) + 1;
//     if (len < len_msg) return FALSE;
//     led_struct *led;
//     for (int i = 0; i < led_count; i++)
//     {
//         led = &buff[i*sizeof(led_struct) + 1];
//     }
// }