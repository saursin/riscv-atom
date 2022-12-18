/* 
    ========================================
            Startup code for Atom
    ========================================
*/

.section .boot, "ax", @progbits
.global _start
.global _exit

_start:
    # ===== initialization =====

    # initialize sp & gp
    la sp, _stack_pointer       # set stack pointer
    la gp, _global_pointer      # set global pointer

    # ===== Call main =====
    jal main


_exit:
    ebreak  # Exit simulation
    j _exit
