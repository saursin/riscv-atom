 .global _start
_start:

li t0, 0x00d01010
li t1, 0x1ddc1044
li t2, 0xdeadbeef
li t3, 0x22101301
li t4, 0xfaf01569
li t5, 0x078b102a
li t6, 0xdae013c0

addi a0, t0, 0x044
addi a1, t1, 0x5ef
addi a2, t2, 0x301
addi a3, t3, 0x569
addi a4, t4, 0x02a
addi a5, t5, 0x3c0

nop
nop
ebreak

# $-ASSERTIONS-$
# eq a0 0x00d01054
# eq a1 0x1ddc1633
# eq a2 0xdeadc1f0
# eq a3 0x2210186a
# eq a4 0xfaf01593
# eq a5 0x078b13ea