.global _start
_start:

    andi t0, t0, 0 # Clear register t0
    andi t1, t1, 0 # Clear register t1

    li t0, 0x00001000     # Load register t0 with an address

    li t1, 0x7155         # Load register t1 with a 2-Byte value
    sh t1, 0(t0)          # Store the byte in t1 into first Halfword slot of address specified in t0

    li t2, 0x8955         # Load register t1 with another 2-Byte value
    sh t2, 2(t0)          # Store the byte in t1 into second halfword slot of address specified in t0

    lh a0, 0(t0)
    lh a1, 2(t0)
    lhu a2, 2(t0)

    li t3, 0x9999   # misaligned store overwriting previous value in memory partially
    sh t3, 1(t0)

    lh a3, 0(t0)
    lh a4, 1(t0)
    lh a5, 2(t0)
    lhu a6, 2(t0)


    nop
    ebreak

# $-ASSERTIONS-$
# eq t0 0x00001000
# eq t1 0x00007155
# eq t2 0x00008955
# eq a0 0x00007155
# eq a1 0xFFFF8955
# eq a2 0x00008955

# eq t3 0x00009999
# eq a3 0xFFFF9955
# eq a4 0xFFFF9999
# eq a5 0xFFFF8999
# eq a6 0x00008999