.global _start
_start:
li t0, 0xffffaaaa
li t1, 0x0000aaaa
li t2, 0xfafafafa
li t3, 0xfbcdefa1

li s0, 1
srl a0, t0, s0
li s0, 0
srl a1, t1, s0
li s0, 5
srl a2, t2, s0
li s0, 31
srl a3, t3, s0

nop
nop
ebreak
