.global _start
_start:

    andi t0, t0, 0 # Clear register t0
    andi t1, t1, 0 # Clear register t1

    la t0, _sdata    # Load register t0 with an address

    li t1, 0x7155         # Load register t1 with a 2-Byte value
    sh t1, 0(t0)          # Store the byte in t1 into first Halfword slot of address specified in t0

    li t2, 0x8955         # Load register t1 with another 2-Byte value
    sh t2, 2(t0)          # Store the byte in t1 into second halfword slot of address specified in t0

    lh a0, 0(t0)
    lh a1, 2(t0)
    lhu a2, 2(t0)

    lw s0, 0(t0)

    li t3, 0xcc     # load a 1 byte value
    sb t3, 1(t0)
    li t3, 0xdd     # load a 1 byte value
    sb t3, 2(t0)

    lh a3, 0(t0)
    lh a4, 2(t0)
    lw a5, 0(t0)
    lhu a6, 2(t0)

    nop
    ebreak
