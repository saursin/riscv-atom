 .global _start
_start:

li t0, 0x12340000
li t1, 0x4e2dfe12



mulhsu a0,t0,t0
mulhsu a1,t1,t1
mulhsu a2,t0,t1

nop
nop
ebreak
