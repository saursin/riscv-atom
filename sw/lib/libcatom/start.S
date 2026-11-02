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
    mv s1, a1                   # platform table from the bootloader (kept across the calls below)
    mv s2, ra                   # where the bootloader wants us back on exit (see _exit)

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

    // after the bss clear: save the platform table pointer and the return address
    la t0, __platform
    sw s1, 0(t0)
    la t0, boot_return
    sw s2, 0(t0)

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

    # ===== The rest in C (libc_start.c): heap, stdio, constructors, main, exit =====
    la a0, main
    j __libc_start_main

# Back to the bootloader (its console) if it started us, i.e. ra pointed into the boot ROM;
# otherwise stay here. a0: the exit code, passed on.
_exit:
    mv s1, a0
    jal serial_flush        # let the UART finish sending
    slti x0, x0, 1          # exit hint: atomsim stops here, NOP on hardware
    mv a0, s1
    la t0, boot_return
    lw t0, 0(t0)
    li t1, SOC_BOOTROM_ADDR
    sub t1, t0, t1
    li t2, SOC_BOOTROM_SIZE
    bgeu t1, t2, 1f
    jr t0
1:  j 1b


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


# ra at _start: where the bootloader wants the program back (_exit). In .bss, so written after
# the bss clear.
.section .bss
.p2align 2
boot_return:
    .zero 4
