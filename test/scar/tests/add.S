.global _start
_start:

li t0, 0x00d01010
li t1, 0x1ddc1044
li t2, 0xdeadbeef
li t3, 0x22101301
li t4, 0xfaf01569
li t5, 0x078b102a
li t6, 0xdae013c0


add a0, t0, t1
add a1, t1, t2
add a2, t2, t3
add a3, t3, t4
add a4, t4, t5
add a5, t5, t6

nop
nop
ebreak
