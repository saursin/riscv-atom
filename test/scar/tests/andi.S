 .global _start
_start:

li t0, 0x00d01010
li t1, 0x1ddc1044
li t2, 0xdeadbeef
li t3, 0x22101301
li t4, 0xfaf01569
li t5, 0x078b102a


andi a0, t0, 0x11a
andi a1, t1, 0x22c
andi a2, t2, 0x46b
andi a3, t3, 0x75d
andi a4, t4, 0x32e
andi a5, t5, 0x00f

nop
nop
ebreak
