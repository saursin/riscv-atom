.global _start
_start:
li t0, 24
li t1, 99
li t2, -1
li t3, -444

slt a0, t0, 99
slt a1, t2, -1
slt a2, t1, 99
slt a3, t3, 24

nop
nop
ebreak
