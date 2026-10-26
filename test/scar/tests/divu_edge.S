    .global _start
_start:
    # ---- divide by zero ----
    li   t0, 0x12345678
    li   t1, 0
    divu s1, t0, t1      # all ones
    remu s2, t0, t1      # dividend

    # ---- extremes ----
    li   t0, 0xFFFFFFFF
    li   t1, 1
    divu s3, t0, t1      # 0xFFFFFFFF
    li   t1, 0xFFFFFFFF
    divu s4, t0, t1      # 1

    # ---- dividends around byte boundaries (divider leading-zero skip) ----
    li   t0, 0x00FFFFFF
    li   t1, 0x10
    divu s5, t0, t1      # 0x000FFFFF
    li   t0, 0x01000000
    li   t1, 3
    remu s6, t0, t1      # 1
    li   t0, 0x0000FFFF
    li   t1, 0x101
    divu s7, t0, t1      # 0xFF
    li   t0, 0x00010000
    li   t1, 7
    remu s8, t0, t1      # 2
    li   t0, 0
    li   t1, 5
    divu s9, t0, t1      # 0

    nop
    nop
    ebreak

#>>>ASSERT
# s1 == 0xFFFFFFFF    # x / 0 = all ones
# s2 == 0x12345678    # x % 0 = x
# s3 == 0xFFFFFFFF    # 0xFFFFFFFF / 1
# s4 == 0x00000001    # 0xFFFFFFFF / 0xFFFFFFFF
# s5 == 0x000FFFFF    # 0x00FFFFFF / 0x10
# s6 == 0x00000001    # 0x01000000 % 3
# s7 == 0x000000FF    # 0x0000FFFF / 0x101
# s8 == 0x00000002    # 0x00010000 % 7
# s9 == 0x00000000    # 0 / 5
#<<<END_ASSERT
