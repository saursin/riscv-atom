 .global _start
_start:

li t0, 0x12340000
li t1, 0x00456123



mulh a0,t0,t0
mulh a1,t0,t1
mulh a2,t1,t1

nop
nop
ebreak
