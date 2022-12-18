#ifndef __TIME_H__
#define __TIME_H__
#include "platform.h"

typedef unsigned long long int clock_t;

/**
 * @brief Get current CPU ticks (uses cycle csr register)
 * @return clock_t 
 */
 clock_t cycle();


/**
 * @brief sleep for time specified in milliseconds
 * 
 * @param ms time (in ms)
 */
void sleep_ms(clock_t ms);


/**
 * @brief sleep for time specified in microseconds
 * 
 * @param ms time (in us)
 */
void sleep_us(clock_t ms);


#endif //__TIME_H__