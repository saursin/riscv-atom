.global _start

_start:
    li a0, 0xdeadbeef
    li a1, 0xbabecafe
    li a2, 0x10101010
    li a3, 0xabcbedfe
    
    la t0, subr
    jalr t0
    li a4, 0x1b0b1d0d
    nop
    nop
    ebreak

subr:
    add a0, a0, 1
    add a1, a1, 1
    add a2, a2, 1
    add a3, a3, 1
    ret
