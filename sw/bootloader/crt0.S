// ===== Startup code for bootloader =====
.section .boot, "ax", @progbits
.global _start
.global _reenter
.global _exit

_start:
.option norvc
    li  s0, 0               // main(BOOT_POWERON, 0)
    li  s1, 0
    j   init

// A program started by the bootloader returns here from its _exit (ra), with its exit code in
// a0. The program used the bootloader's RAM, so set it up again, then main(BOOT_REENTRY, code).
_reenter:
    li  s0, 1               // main(BOOT_REENTRY, return code)
    mv  s1, a0

init:
    // Initialize sp & gp
    la sp, _stack_pointer
.option push
.option norelax
    la gp, _global_pointer
.option pop
    
    // copy data from rom to ram
    la	a0, _sdata      // destination
    la	a1, _etext      // source
    la  t0, _edata
    sub a2, t0, a0      // size = _edata - _sdata
	jal memcpy
    
    // zero initialize bss section
    la	a0, _sbss       // destination
	li	a1, 0           // value = 0
	la  t0, _ebss
    sub a2, t0, a0      // size = _sbss - _sbss
	jal	memset

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

    // Call main(boot kind, parameter): see BOOT_* in common.h
    mv  a0, s0
    mv  a1, s1
    jal main


_exit:
    jal serial_flush        // let the UART finish sending
    slti x0, x0, 1          // exit hint: atomsim stops here, NOP on hardware
1:  j 1b
