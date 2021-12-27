#ifndef __TIME_H__
#define __TIME_H__

#define SW_SLEEP
// #define TIMER_SLEEP

typedef unsigned long int clock_t;

#define DELAY_SCALE_FACTOR 1200

/**
 * @brief sleep for some specified duration
 * 
 * @param count time (in ms)
 */
void sleep(long unsigned int count);

#endif //__TIME_H__