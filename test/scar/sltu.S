.global _start
_start:
li t0, 24
li t1, 99
li t2, -1
li t3, -444

sltu a0, t0, t1
sltu a1, t2, t3
sltu a2, t1, t2
sltu a3, t3, t0

nop
nop
ebreak

# $-ASSERTIONS-$
# eq a0 0x00000001
# eq a1 0x00000000
# eq a2 0x00000001
# eq a3 0x00000000
