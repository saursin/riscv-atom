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

	// Initialize heap
	jal heap_init

	/* Disable and clear all interrupt sources */
    li   a3, -1
    csrc mie, a3
    csrc mip, a3

    # Initialize MTVEC
    la t0, trap_handler_base
    csrw mtvec, t0

    # Enable Interrupts Globally
    li t0, 0b1000
    csrw mstatus, t0

    # ===== Call main =====
    li  a0, 0
    li  a1, 0
    jal main

_exit:
#ifdef SIM
	ebreak  # Exit simulation
#endif
    j _exit


.macro PROLOGUE
	addi	sp, sp, -96		# - size(context_frame) * 4
	sw	t6, 68(sp)
	sw	t5, 64(sp)
	sw	t4, 60(sp)
	sw	t3, 56(sp)
	sw	a7, 52(sp)
	sw	a6, 48(sp)
	sw	a5, 44(sp)
	sw	a4, 40(sp)
	sw	a3, 36(sp)
	sw	a2, 32(sp)
	sw	a1, 28(sp)
	sw	a0, 24(sp)
	sw	t2, 20(sp)
	sw	t1, 16(sp)
	sw	t0, 12(sp)
	sw	ra, 8(sp)
	csrr	t0, mstatus
	sw	t0, 4(sp)
	csrr	t0, mepc
	sw	t0, 0(sp)
.endm

.macro EPILOGUE
	lw	t0, 0(sp)
	csrw	mepc, t0
	lw	t0, 4(sp)
	csrw	mstatus, t0
	lw	ra, 8(sp)
	lw	t0, 12(sp)
	lw	t1, 16(sp)
	lw	t2, 20(sp)
	lw	a0, 24(sp)
	lw	a1, 28(sp)
	lw	a2, 32(sp)
	lw	a3, 36(sp)
	lw	a4, 40(sp)
	lw	a5, 44(sp)
	lw	a6, 48(sp)
	lw	a7, 52(sp)
	lw	t3, 56(sp)
	lw	t4, 60(sp)
	lw	t5, 64(sp)
	lw	t6, 68(sp)
	addi	sp, sp, 96		# size(context_frame) * 4
.endm


.p2align 2					# Needs 4Byte Alignment, refer mtvec reg
trap_handler_base:
	PROLOGUE
	csrr	a0, mcause
    sw	    a0, 72(sp)
	mv	    a1, sp
	jal		trap_handler	# call exception handler in C
	EPILOGUE
	mret

# .weak trap_handler 			# Can be overridden
# trap_handler:
#     ret