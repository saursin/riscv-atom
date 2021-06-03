.global _start

_start:
andi t0, t0, 0 # Clear register t0
andi t1, t1, 0 # Clear register t1
andi t2, t2, 0 # Clear register t2

li t0, 0xFF # Load a 8-bit number to t0
li t1, 0xFFFF # Load a 16-bit number to t1
li t2, 0xFFFFF343 # Load a 32-bit number to t2
li x3, 0x01234567

nop
ebreak
# $-ASSERTIONS-$
# eq t0 0x000000FF
# eq t1 0x0000FFFF
# eq t2 0xFFFFF343
# eq x3 0x01234567
