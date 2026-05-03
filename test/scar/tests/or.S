.global _start
_start:

li t0, 0x00d01010
li t1, 0x1ddc1044
li t2, 0xdeadbeef
li t3, 0x22101301
li t4, 0xfaf01569
li t5, 0x078b102a
li t6, 0xdae013c0

or a0, t0, t1
or a1, t1, t2
or a2, t2, t3
or a3, t3, t4
or a4, t4, t5
or a5, t5, t6
or a6, t6, x0
or a7, a4, a5

nop
nop
ebreak

#>>>ASSERT
# a0 == 0x1ddc1054
# a1 == 0xdffdbeef
# a2 == 0xfebdbfef
# a3 == 0xfaf01769
# a4 == 0xfffb156b
# a5 == 0xdfeb13ea
# a6 == 0xdae013c0
# a7 == 0xfffb17eb
#<<<END_ASSERT
