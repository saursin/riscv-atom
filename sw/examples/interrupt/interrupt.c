#include <stdio.h>
#include <arch.h>
#include <timer.h>

#define TIMER_INTERRUPT_DELAY 1000

void timer_interrupt_handler(){
    // Clear interrupt
    timer_clear_interrupt();
    puts("!!! Caught timer interrupt !!!\n");
}

void setup_timer_interrupt() {
    // Register Interrupt handler
    register_interrupt_handler(7, timer_interrupt_handler);

    // Request timer interrupt
    timer_get_interrupt(TIMER_INTERRUPT_DELAY);

    // Enable interrupts in core
    arch_en_mti();
    arch_en_int();
}

void main()
{
    serial_init(UART_BAUD_115200);
    puts("Before interrupt\n");
    
    setup_timer_interrupt();
    asm volatile("wfi");        // wait for interrupt

    puts("After interrupt\n");
    return;
}
