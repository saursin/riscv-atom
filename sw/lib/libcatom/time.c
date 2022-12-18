#include <stdint.h>
#include "csr.h"
#include "time.h"

#define CLOCKS_PER_SEC CLK_FREQ

clock_t cycle()
{
    union 
    {
        uint64_t uint64;
        uint32_t uint32[sizeof(uint64_t)/2];
    } cycles;

    register uint32_t tmp1, tmp2, tmp3;
    while(1)
    {
        tmp1 = CSR_read(CSR_CYCLEH);
        tmp2 = CSR_read(CSR_CYCLE);
        tmp3 = CSR_read(CSR_CYCLEH);
        if (tmp1 == tmp3) 
        {
            break;
        }
    }

    cycles.uint32[0] = tmp2;
    cycles.uint32[1] = tmp3;

    return cycles.uint64;
}


void sleep_ms(clock_t ms)
{
    clock_t tend = cycle() + (ms * (CLOCKS_PER_SEC/1000));
    while(cycle() < tend) {
        asm volatile("");
    }
}


void sleep_us(clock_t ms)
{
    clock_t tend = cycle() + (ms * (CLOCKS_PER_SEC/1000000));
    while(cycle() < tend) {
        asm volatile("");
    }
}