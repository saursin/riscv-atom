 .global _start
_start:

li t0, 0x12340000
li t1, 0x4e2dfe12



mulhsu a0,t0,t0
mulhsu a1,t1,t1
mulhsu a2,t0,t1

nop
nop
ebreak

#>>>ASSERT
# a0 == 0x014b5a90
# a1 == 0x17e00f16
# a2 == 0x058f1d34
#<<<END_ASSERT
