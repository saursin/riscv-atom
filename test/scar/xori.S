 .global _start
_start:

li t0, 0x00d01010
li t1, 0x1ddc1044
li t2, 0xdeadbeef
li t3, 0x22101301
li t4, 0xfaf01569
li t5, 0x078b102a


xori a0, t0, 0x11a
xori a1, t1, 0x22c
xori a2, t2, 0x46b
xori a3, t3, 0x75d
xori a4, t4, 0x32e
xori a5, t5, 0x00f

nop
nop
ebreak

# $-ASSERTIONS-$
# eq a0 0x00d0110a
# eq a1 0x1ddc1268
# eq a2 0xdeadba84
# eq a3 0x2210145c
# eq a4 0xfaf01647
# eq a5 0x078b1025
