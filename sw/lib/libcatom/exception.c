#include <stdint.h>
#include "stdio.h"
#include "csr.h"
#include "mmio.h"

typedef struct context_frame
{
	uint32_t mepc, mstatus, ra, t0,
		 t1, t2, a0, a1, a2, a3,
		 a4, a5, a6, a7, t3, t4,
		 t5, t6, mcause, pad[5];
		 /* Padding is necessary for alignment */
} context_frame_t;


void common_trap_handler(uint32_t mcause, context_frame_t *frame)
{
    uint32_t cause = mcause & ~(1U << 31);
    if(bitcheck(mcause, 31))
    {
        char intr_cause_str [12][30] = {
            "Reserved",
            "Supervisor software interrupt",
            "Reserved",
            "Machine software interrupt",
            "Reserved",
            "Supervisor timer interrupt",
            "Reserved",
            "Machine timer interrupt",
            "Reserved",
            "Supervisor external interrupt",
            "Reserved",
            "Machine external interrupt"
        };

        // Print interrupt info
        printf("\n>> Caught interrupt [except_code = 0x%x (%s), mepc = 0x%8x]\n", cause, intr_cause_str[cause], CSR_read(CSR_MEPC));
    }
    else    // Exception
    {
        char exception_str [16][30] = {
            "Instruction address misaligned",
            "Instruction access fault",
            "Illegal instruction",
            "Breakpoint",
            "Load address misaligned",
            "Load access fault",
            "Store/AMO address misaligned",
            "Store/AMO access fault",
            "Environment call from U-mode",
            "Environment call from S-mode",
            "Reserved",
            "Environment call from M-mode",
            "Instruction page fault",
            "Load page fault",
            "Reserved",
            "Store/AMO page fault"
        };

        // Print exception Info
        printf("\n>> Caught exception [except_code = %x (%s), mepc = 0x%8x]\n", cause,exception_str[cause], CSR_read(CSR_MEPC));

        if(mcause & (1U << 31)) {
    		// irqhandler[cpuid][cause]();
        }
        else
        {
            // exhandler[cpuid][cause]();
            frame->mepc += (REG32(frame->mepc, 0) & 0x3) ? 4 : 2;
        }
    }
    putchar('\n');
}