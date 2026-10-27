.global _start

_start:
    li a0, 0x55CF
    li a1, 0xFCCD

    jal function

    addi a0, a0, 89
    nop
    slti x0, x0, 1    # exit hint: atomsim stops here

function:
    add a0, a0, a1
    ret

#>>>ASSERT
# a0 == 0x000152f5
# a1 == 0x0000fccd
#<<<END_ASSERT
