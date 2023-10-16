.global _start

_start:
    li a0, 0x55CF
    li a1, 0xFCCD

    jal function

    addi a0, a0, 89
    nop
    ebreak

function:
    add a0, a0, a1
    ret
