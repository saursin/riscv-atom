	.file	"recursion.c"
	.option nopic
	.attribute arch, "rv32i2p0"
	.attribute unaligned_access, 0
	.attribute stack_align, 16
	.text
	.align	2
	.globl	_start
	.type	_start, @function
_start:
	li sp, 0x1000
	addi	sp,sp,-16
	sw	ra,12(sp)
	sw	s0,8(sp)
	addi	s0,sp,16
	li	a0,5
	call	fact
	nop
	mv	a0,a5
	lw	ra,12(sp)
	lw	s0,8(sp)
	addi	sp,sp,16
	
	nop
	nop
	ebreak

	.size	_start, .-_start
	.globl	__mulsi3
	.align	2
	.globl	fact
	.type	fact, @function
fact:
	addi	sp,sp,-32
	sw	ra,28(sp)
	sw	s0,24(sp)
	addi	s0,sp,32
	sw	a0,-20(s0)
	lw	a5,-20(s0)
	bgt	a5,zero,.L3
	li	a5,1
	j	.L4
.L3:
	lw	a5,-20(s0)
	addi	a5,a5,-1
	mv	a0,a5
	call	fact
	mv	a5,a0
	lw	a1,-20(s0)
	mv	a0,a5
	call	__mulsi3
	mv	a5,a0
.L4:
	mv	a0,a5
	lw	ra,28(sp)
	lw	s0,24(sp)
	addi	sp,sp,32
	jr	ra
	.size	fact, .-fact
	.ident	"GCC: (GNU) 10.2.0"


# $-ASSERTIONS-$
# eq a0 0x00000078