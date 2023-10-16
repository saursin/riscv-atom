.global _start
_start:
li t0, 0xffffaaaa
li t1, 0x0000aaaa
li t2, 0xfafafafa
li t3, 0xfbcdefa1

srli a0, t0, 1
srli a1, t1, 0
srli a2, t2, 5
srli a3, t3, 31

nop
nop
ebreak
