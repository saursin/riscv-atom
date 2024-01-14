 .global _start
_start:

li t0, 0x12340000
li t1, 0x45890010



mulhsu a0,t1,t1
mulhsu a1,t0,t0
mulhsu a2,t0,t1

nop
nop
ebreak
