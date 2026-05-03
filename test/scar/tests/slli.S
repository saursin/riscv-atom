.global _start
_start:
li t0, 0xdeadbeef
li t1, 0xbabecafe
li t2, 0x1234bcda
li t3, 0x41421231

slli t0, t0, 0
slli t1, t1, 1
slli t2, t2, 14
slli t3, t3, 31

nop
nop
ebreak

#>>>ASSERT
# t0 == 0xdeadbeef
# t1 == 0x757d95fc
# t2 == 0x2f368000
# t3 == 0x80000000
#<<<END_ASSERT
