#ifndef __TIME_H__
#define __TIME_H__
#include "platform.h"

#define SW_SLEEP
// #define TIMER_SLEEP

typedef unsigned long long int clock_t;

#define DELAY_SCALE_FACTOR 1200

#define CLOCKS_PER_SEC CLK_FREQ

/**
 * @brief sleep for some specified duration
 * 
 * @param count time (in ms)
 */
void sleep(long unsigned int count);

/**
 * @brief Get current CPU ticks
 * 
 * @return clock_t 
 */
 clock_t cycle();

#endif //__TIME_H__