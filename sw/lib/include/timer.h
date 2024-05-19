#pragma once
#include <stdint.h>

uint64_t timer_get_time();

void timer_get_interrupt(uint64_t time);

void timer_clear_interrupt();
