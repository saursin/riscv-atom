.global _start
_start:
li t0, 24
li t1, 99
li t2, -1
li t3, -444

sltiu a0, t0, 99
sltiu a1, t2, -444
sltiu a2, t1, -1
sltiu a3, t3, 24

nop
nop
ebreak

# $-ASSERTIONS-$
# eq a0 0x00000001
# eq a1 0x00000000
# eq a2 0x00000001
# eq a3 0x00000000
