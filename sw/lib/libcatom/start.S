/* 
    ========================================
          Startup code for HydrogenSoC
    ========================================
*/
.section .boot, "ax", @progbits
.global _start
.global _exit

_start:
.option norvc
    # ===== initialization =====
    # initialize sp & gp
    la sp, _stack_pointer       # set stack pointer
.option push
.option norelax
    la gp, _global_pointer      # set global pointer
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

    # ===== Call main =====
    li  a0, 0
    li  a1, 0
    jal main

_exit:
    ebreak  # Exit simulation
    j _exit
