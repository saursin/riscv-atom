    .global _start
_start:
    # ---- overflow: INT_MIN / -1 ----
    li   t0, 0x80000000
    li   t1, -1
    div  s1, t0, t1      # INT_MIN
    rem  s2, t0, t1      # 0

    # ---- divide by zero, negative dividend ----
    li   t0, -7
    li   t1, 0
    div  s3, t0, t1      # -1
    rem  s4, t0, t1      # dividend

    # ---- sign combinations (quotient truncates, remainder takes dividend sign) ----
    li   t0, -7
    li   t1, 2
    div  s5, t0, t1      # -3
    rem  s6, t0, t1      # -1
    li   t0, 7
    li   t1, -2
    div  s7, t0, t1      # -3
    rem  s8, t0, t1      # 1
    li   t0, -7
    li   t1, -2
    div  s9, t0, t1      # 3
    rem  s10, t0, t1     # -1

    # ---- dependent results (forwarding of division results) ----
    li   t0, 100
    li   t1, 7
    div  t2, t0, t1      # 14
    addi t3, t2, 1       # 15
    div  t4, t2, t1      # 14 / 7 = 2
    rem  t5, t0, t1      # 2

    nop
    nop
    ebreak

#>>>ASSERT
# s1  == 0x80000000    # INT_MIN / -1 = INT_MIN
# s2  == 0x00000000    # INT_MIN % -1 = 0
# s3  == 0xFFFFFFFF    # -7 / 0 = -1
# s4  == 0xFFFFFFF9    # -7 % 0 = -7
# s5  == 0xFFFFFFFD    # -7 / 2 = -3
# s6  == 0xFFFFFFFF    # -7 % 2 = -1
# s7  == 0xFFFFFFFD    # 7 / -2 = -3
# s8  == 0x00000001    # 7 % -2 = 1
# s9  == 0x00000003    # -7 / -2 = 3
# s10 == 0xFFFFFFFF    # -7 % -2 = -1
# t2  == 0x0000000E    # 100 / 7 = 14
# t3  == 0x0000000F    # dependent addi
# t4  == 0x00000002    # dependent div
# t5  == 0x00000002    # 100 % 7 = 2
#<<<END_ASSERT
