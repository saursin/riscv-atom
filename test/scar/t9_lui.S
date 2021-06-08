.global _start
_start:

lui a0, 0xfffff


sub a0, t0, t1
sub a1, t1, t2
sub a2, t2, t3
sub a3, t3, t4
sub a4, t4, t5
sub a5, t5, t6

nop
nop
ebreak

# $-ASSERTIONS-$
# eq a0 0xfffff000
# eq a1 0x3f2e5155
# eq a2 0xdeadabee
# eq a3 0x050ffd98
# eq a4 0xf365053f
# eq a5 0x2caafc6a