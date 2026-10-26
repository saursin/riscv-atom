    .global _start
_start:
    li     t0, -1
    li     t1, 0x80000000
    li     t2, 0xFFFFFFFF
    li     t3, 0x12345678
    li     t4, 0x9ABCDEF0

    mulh   s1, t0, t0    # (-1)*(-1) = 1          -> high 0
    mulh   s2, t1, t1    # MIN*MIN = 2^62         -> high 0x40000000
    mulhu  s3, t2, t2    # (2^32-1)^2             -> high 0xFFFFFFFE
    mulhsu s4, t0, t2    # (-1)*(2^32-1)          -> high 0xFFFFFFFF
    mulhsu s5, t1, t2    # MIN*(2^32-1)           -> high 0x80000000
    mul    s6, t1, t0    # MIN*(-1) low           -> 0x80000000
    mulh   s7, t1, t0    # MIN*(-1) = 2^31        -> high 0
    mulhu  s8, t3, t4    # unsigned*unsigned
    mulh   s9, t3, t4    # signed*signed (t4 negative)
    mulhsu s10, t4, t3   # signed*unsigned (t4 negative)

    nop
    nop
    ebreak

#>>>ASSERT
# s1  == 0x00000000
# s2  == 0x40000000
# s3  == 0xFFFFFFFE
# s4  == 0xFFFFFFFF
# s5  == 0x80000000
# s6  == 0x80000000
# s7  == 0x00000000
# s8  == 0x0B00EA4E
# s9  == 0xF8CC93D6
# s10 == 0xF8CC93D6
#<<<END_ASSERT
