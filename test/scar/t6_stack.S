.global _start
_start:

li sp, 0x1000
li t0, 0x43234323
li t1, 0x443c
li t2, 0xde

addi sp, sp, -16
sw t0, 0(sp)
sw t1, 4(sp)
sw t2, 8(sp)

addi sp, sp, -48
addi sp, sp, 48

lw a0, 0(sp)
lw a1, 4(sp)
lw a2, 8(sp)

nop
nop
ebreak

# $-ASSERTIONS-$
# eq a0 0x43234323
# eq a1 0x0000443C
# eq a2 0x000000DE