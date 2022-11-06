/* 
    ========================================
          Startup code for HydrogenSoC
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

    // copy data from rom to ram
    la	a0, _sdata      // destination
    la	a1, _etext      // source
    la  t0, _edata
    sub a2, t0, a0      // size = _edata - _sdata
	call    memcpy
    
    // zero initialize bss section
    la	a0, _sbss       // destination
	li	a1, 0           // value = 0
	la  t0, _ebss
    sub a2, t0, a0      // size = _sbss - _sbss
	call	memset

    # ===== Call main =====
    li  a0, 0
    li  a1, 0
    jal main

_exit:
    ebreak  # Exit simulation
    j _exit


/* 
    ========================================
            Interrupt vector Table                
    ========================================

    for reference, see https://github.com/danielinux/riscv-boot
*/

/* Utility macros */
.macro trap_entry
    addi sp, sp, -64
    sw x1,  0(sp)
    sw x5,  4(sp)
    sw x6,  8(sp)
    sw x7,  12(sp)
    sw x10, 16(sp)
    sw x11, 20(sp)
    sw x12, 24(sp)
    sw x13, 28(sp)
    sw x14, 32(sp)
    sw x15, 36(sp)
    sw x16, 40(sp)
    sw x17, 44(sp)
    sw x28, 48(sp)
    sw x29, 52(sp)
    sw x30, 56(sp)
    sw x31, 60(sp)
.endm

.macro trap_exit
  lw x1,  0(sp)
  lw x5,  4(sp)
  lw x6,  8(sp)
  lw x7,  12(sp)
  lw x10, 16(sp)
  lw x11, 20(sp)
  lw x12, 24(sp)
  lw x13, 28(sp)
  lw x14, 32(sp)
  lw x15, 36(sp)
  lw x16, 40(sp)
  lw x17, 44(sp)
  lw x28, 48(sp)
  lw x29, 52(sp)
  lw x30, 56(sp)
  lw x31, 60(sp)
  addi sp, sp, 64
  mret
.endm



/*
    Interrupt Vector Table (IVT)
    This table contains 32 entries each jumping to irs respective ISR.
    This table is loaded into the .vector section.
*/
.section .vector

.global _IVT
_IVT:
    j trap_empty
    .align 2
    j trap_empty
    .align 2
    j trap_empty
    .align 2
    j trap_empty
    .align 2
    j trap_empty
    .align 2
    j trap_empty
    .align 2
    j trap_empty
    .align 2
    j trap_empty
    .align 2
    j trap_empty
    .align 2
    j trap_empty
    .align 2
    j trap_empty
    .align 2
    j trap_empty
    .align 2
    j trap_empty
    .align 2
    j trap_empty
    .align 2
    j trap_empty
    .align 2
    j trap_empty
    .align 2
    j trap_empty
    .align 2
    j trap_empty
    .align 2
    j trap_empty
    .align 2
    j trap_empty
    .align 2
    j trap_empty
    .align 2
    j trap_empty
    .align 2
    j trap_empty
    .align 2
    j trap_empty
    .align 2
    j trap_empty
    .align 2
    j trap_empty
    .align 2
    j trap_empty
    .align 2
    j trap_empty
    .align 2
    j trap_empty
    .align 2
    j trap_empty
    .align 2
    j trap_empty
    .align 2
    j trap_empty
    .align 2


/* 
    By default all vectored interrupts jump to trap_empty.
    to change this define a new trap label, followed by the following:
    1. trap_entry macro
    2. call to trap handler function (implemented in c/asm)
    3. trap_exit macro
*/


# Default Trap Handler
trap_empty:
    j trap_empty
