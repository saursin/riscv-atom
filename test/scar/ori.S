 .global _start
_start:

li t0, 0x00d01010
li t1, 0x1ddc1044
li t2, 0xdeadbeef
li t3, 0x22101301
li t4, 0xfaf01569
li t5, 0x078b102a


ori a0, t0, 0x11a
ori a1, t1, 0x22c
ori a2, t2, 0x46b
ori a3, t3, 0x75d
ori a4, t4, 0x32e
ori a5, t5, 0x00f

nop
nop
ebreak

# $-ASSERTIONS-$

# eq a0 0x00d0111a
# eq a1 0x1ddc126c
# eq a2 0xdeadbeef
# eq a3 0x2210175d
# eq a4 0xfaf0176f
# eq a5 0x078b102f
