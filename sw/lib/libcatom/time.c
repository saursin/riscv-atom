#include "time.h"

/**
 * @brief sleep for some specified duration
 * 
 * @param count time (in ms)
 */
void sleep(long unsigned int count)
{
    #ifdef SW_SLEEP
    count = count*DELAY_SCALE_FACTOR;
    while(count-->0);
    #endif
}

/**
 * @brief Get current CPU ticks
 * 
 * @return clock_t 
 */
clock_t clock()
{
    clock_t timel, timeh1, timeh2;
    do
    {
        __asm__ volatile ("rdcycleh %0" :"=r"(timeh1):);
        __asm__ volatile ("rdcycle  %0" :"=r"(timel):);
        __asm__ volatile ("rdcycleh %0" :"=r"(timeh2):);
    } while(timeh1!=timeh2);

    clock_t time;
    __asm__ volatile ("rdcycle %0" :"=r"(time));
    // __asm__ volatile ("csrrs %0, cycle, zero" :"=r"(time));

    return (timeh1 << 32) | timel;
}