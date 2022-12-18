#include <stdint.h>
#include "csr.h"
#include "time.h"

void sleep(long unsigned int count)
{
    #ifdef SW_SLEEP
    count = count*DELAY_SCALE_FACTOR;
    while(count-->0);
    #endif
}


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