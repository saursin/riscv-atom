#include <timer.h>
#include <mmio.h>
#include <platform.h>

#define TIMER_REG_MTIME         0x0
#define TIMER_REG_MTIMEH        0x4
#define TIMER_REG_MTIMECMP      0x8
#define TIMER_REG_MTIMECMPH     0xc

uint64_t timer_get_time() {
    uint64_t time = (uint64_t) REG32(TIMER_ADDR, TIMER_REG_MTIMEH);
    time = (time << 32 ) | (uint64_t) REG32(TIMER_ADDR, TIMER_REG_MTIME);
    return time;
}

void timer_get_interrupt(uint64_t time){
    uint64_t curtime = (uint64_t) REG32(TIMER_ADDR, TIMER_REG_MTIMEH);
    curtime = (curtime << 32 ) | (uint64_t) REG32(TIMER_ADDR, TIMER_REG_MTIME);
    uint64_t exp_time = curtime + time;
    REG32(TIMER_ADDR, TIMER_REG_MTIMECMPH) = 0xffffffff & (exp_time >> 32);
    REG32(TIMER_ADDR, TIMER_REG_MTIMECMP) = 0xffffffff & exp_time;
}

void timer_clear_interrupt(){
    REG32(TIMER_ADDR, TIMER_REG_MTIMECMPH) = (uint32_t)-1;
    REG32(TIMER_ADDR, TIMER_REG_MTIMECMP) = (uint32_t)-1;
}