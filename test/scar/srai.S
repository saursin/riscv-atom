.global _start
_start:
li t0, 0xffffaaaa
li t1, 0x0000aaaa
li t2, 0xfafafafa
li t3, 0xfbcdefa1

srai a0, t0, 1
srai a1, t1, 0
srai a2, t2, 5
srai a3, t3, 31

nop
nop
ebreak

# $-ASSERTIONS-$
# eq a0 0xffffd555
# eq a1 0x0000aaaa
# eq a2 0xffd7d7d7
# eq a3 0xffffffff
