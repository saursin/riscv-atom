.global _start
_start:
    li a2, 0x12345678   // A_LO
    li a3, 0x12345678   // A_HI

    li a0, 0x12345678   // B_LO
    li a1, 0x12345678   // B_HI
    
    add     a4, a2, a0  // S_LO = A_LO + B_LO
    mv      a6,a4
    sltu    a6,a6,a2    // C_LO: get 32 bit carry in a6

    add     a5,a3,a1    // S_HI = A_HI + B_HI
    add     a3,a6,a5    // S_HI += C_LO
    mv      a5,a3

    nop
    nop
    ebreak
