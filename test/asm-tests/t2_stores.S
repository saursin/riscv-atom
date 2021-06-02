.global _start
_start:
    li t0, 0x384
    li t1, 0x100
    sw t0, 0(t1)
    nop
    nop
    nop
    lw ra, 0(t1)
    nop
    nop
    ebreak
