.global _start
_start:
li t0, 0xdeadbeef
li t1, 0xbabecafe
li t2, 0x1234bcda
li t3, 0x41421231

li a0, 0
sll t0, t0, a0
li a0, 1
sll t1, t1, a0
li a0, 14
sll t2, t2, a0
li a0, 31
sll t3, t3, a0

nop
nop
ebreak

# $-ASSERTIONS-$
# eq t0 0xdeadbeef
# eq t1 0x757d95fc
# eq t2 0x2f368000
# eq t3 0x80000000
