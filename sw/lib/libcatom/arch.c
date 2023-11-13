#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <mmio.h>
#include <assert.h>
#include <arch.h>


#define DBG_TRAPS
#ifdef DBG_TRAPS
    #define D(x) x
#else
    #define D(x)
#endif

#define N_IRQS 12
#define N_EXCEPTIONS 16

typedef struct context_frame
{
	uint32_t mepc, mstatus, ra, t0,
		 t1, t2, a0, a1, a2, a3,
		 a4, a5, a6, a7, t3, t4,
		 t5, t6, mcause, pad[5];
		 /* Padding is necessary for alignment */
} context_frame_t;

const char intr_cause_str [12][4] = {
    "Res",     // -
    "SSI",     // Supervisor Software Interrupt
    "Res",     // -
    "MSI",     // Machine Software Interrupt
    "Res",     // -
    "STI",     // Supervisor Timer Interrupt
    "Res",     // -
    "MTI",     // Machine Timer Interrupt
    "Res",     // -
    "SEI",     // Supervisor External Interrupt
    "Res",     // -
    "MEI"      // Machine External Interrupt
};

const char exception_cause_str [16][4] = {
    "IAM",  // Instruction Address Misaligned Fault
    "IAF",  // Instruction Access Fault
    "ILI",  // Illegal Instruction Fault
    "BRK",  // Breakpoint
    "LAM",  // Load Address Misaligned Fault
    "LAF",  // Load Access Fault
    "SAM",  // Store Adress Misaligned Fault
    "SAF",  // Store Access Fault
    "ECU",  // 
    "ECS",  //
    "Res",  //
    "ECM",  //
    "IPF",  // Instruction Page Fault
    "LPF",  // Load Page Fault
    "Res",  // -
    "SPF"   // Store Page Fault
};

static void (* exhandler_table[N_EXCEPTIONS])(void) = {[0 ... N_EXCEPTIONS-1] = arch_unhandled_exception};
static void (* irqhandler_table[N_IRQS])(void) = {[0 ... N_IRQS-1] = arch_unhandled_irq};
static context_frame_t *local_frame;

_WEAK void arch_unhandled_irq()
{
    uint32_t id = local_frame->mcause & ~(1U << 31);
	printf("\n[PANIC]: Unhandled IRQ!: Id = 0x%x (%s)\n", id, intr_cause_str[id]);
	exit(1);
}

_WEAK void arch_unhandled_exception()
{
    uint32_t cause = local_frame->mcause & ~(1U << 31);
	printf("\n[PANIC]: Unhandled Exception!: Cause = 0x%x (%s), Addr = 0x%8x)\n", cause, exception_cause_str[cause], local_frame->mepc);
	exit(1);
}

void register_exception_handler(unsigned id, void (*handler)(void))
{
    assert(id < N_EXCEPTIONS);
	exhandler_table[id] = handler;
}

void deregister_exception_handler(unsigned id)
{
    assert(id < N_EXCEPTIONS);
	exhandler_table[id] = arch_unhandled_exception;
}

void register_interrupt_handler(unsigned id, void (*handler)(void))
{
    assert(id < N_IRQS);
	irqhandler_table[id] = handler;
}

void deregister_interrupt_handler(unsigned id)
{
    assert(id < N_IRQS);
	irqhandler_table[id] = arch_unhandled_irq;
}

void trap_handler(uint32_t mcause, context_frame_t *frame)
{   
    // assumes console is already initialized
    uint32_t cause = mcause & ~(1U << 31);
    local_frame = frame;
    if(bitcheck(mcause, 31))
    {
        // Handle interrupt
        irqhandler_table[cause]();
    }
    else
    {
        // Handle exception
        exhandler_table[cause]();

        // set mepc to next instruction depending on whether its a compressed instruction or normal instruction 
        frame->mepc += (REG8(frame->mepc, 0) & 0x3) ? 4 : 2;
    }
}
