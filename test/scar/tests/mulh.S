.global _start
_start:

li t0, 0x12340000
li t1, 0x00456123



mulh a0,t0,t0
mulh a1,t0,t1
mulh a2,t1,t1

nop
nop
ebreak

#>>>ASSERT
# a0 == 0x014b5a90
# a1 == 0x0004eeec
# a2 == 0x000012cd
#<<<END_ASSERT
