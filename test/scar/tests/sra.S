.global _start
_start:
li t0, 0xffffaaaa
li t1, 0x0000aaaa
li t2, 0xfafafafa
li t3, 0xfbcdefa1

li s0, 1
sra a0, t0, s0
li s0, 0
sra a1, t1, s0
li s0, 5
sra a2, t2, s0
li s0, 31
sra a3, t3, s0

nop
nop
ebreak
