.global _start
_start:
    li a0, 5


fact:
    # n = a0
    bgt a0, x0, notZ
        li a0, 1
        ret
    
    notZ:
        addi a1, a0, -1
        addi sp, sp, -16
        sw ra, 0(sp)
        jal _mulsi3
