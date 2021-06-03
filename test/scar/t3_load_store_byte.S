.global _start
_start:

    andi t0, t0, 0 # Clear register t0
    andi t1, t1, 0 # Clear register t1

    li t0, 0x00001000    # Load register t0 with an address

    li t1, 0x71         # Load register t1 with a 1-Byte value
    sb t1, 0(t0)        # Store the byte in t1 into first byte slot of address specified in t0

    li t2, 0x89         # Load register t1 with another 1-Byte value
    sb t2, 1(t0)        # Store the byte in t1 into second byte slot of address specified in t0

    lb a0, 0(t0)
    lb a1, 1(t0)
    lbu a2, 1(t0)

    nop
    ebreak

# $-ASSERTIONS-$
# eq t0 0x00001000
# eq t1 0x00000071
# eq t2 0x00000089
# eq a0 0x00000071
# eq a1 0xFFFFFF89
# eq a2 0x00000089