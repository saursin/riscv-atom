.global _start
_start:

li t0, 0x43234323
li t1, 0xbabecafe
li t2, 0xdeafbeef

la sp, _stack_pointer

addi sp, sp, -16
sw t0, 0(sp)

addi sp, sp, -32
sw t1, 4(sp)

addi sp, sp, -64
sw t2, 8(sp)

addi sp, sp, -48
addi sp, sp, 48

lw a2, 8(sp)
addi sp, sp, 64

lw a1, 4(sp)
addi sp, sp, 32

lw a0, 0(sp)
addi sp, sp, 16

la s0, _stack_pointer
bne s0, sp, exit
li sp, 0x010a020b

exit:

nop
nop
ebreak
