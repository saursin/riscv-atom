// ===== Startup code for bootloader =====
.section .boot, "ax", @progbits
.global _start
.global _exit

_start:
.option norvc
    // Initialize sp & gp
    la sp, _stack_pointer
.option push
.option norelax
    la gp, _global_pointer
.option pop

#ifdef EN_EXCEPT
	// Disable and clear all interrupt sources
    li   a3, -1
    csrc mie, a3
    csrc mip, a3

    // Initialize mtvec CSR
    la t0, boot_panic_handler
    csrw mtvec, t0

    // Enable interrupts globally
    li t0, 0b1000
    csrw mstatus, t0
#endif

    // Call main function
    li  a0, 0
    li  a1, 0
    jal main


_exit:
    // Exit simulation
    ebreak
    j _exit
