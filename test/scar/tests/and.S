.global _start
_start:

li t0, 0x00d01010
li t1, 0x1ddc1044
li t2, 0xdeadbeef
li t3, 0x22101301
li t4, 0xfaf01569
li t5, 0x078b102a
li t6, 0xdae013c0

and a0, t0, t1
and a1, t1, t2
and a2, t2, t3
and a3, t3, t4
and a4, t4, t5
and a5, t5, t6
and a6, t6, x0
and a7, a4, a5

nop
nop
ebreak
