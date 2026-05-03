 .global _start
_start:

li t0, 0x12340000
li t1, 0x45890010



mulhsu a0,t1,t1
mulhsu a1,t0,t0
mulhsu a2,t0,t1

nop
nop
ebreak

#>>>ASSERT
# a0 == 0x12e32359
# a1 == 0x014b5a90
# a2 == 0x04f1c1d5
#<<<END_ASSERT
