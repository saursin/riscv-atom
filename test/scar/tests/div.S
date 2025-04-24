    .global _start
_start:
    li t0, 0xFFF6E19B    # -597605
    li t1, 0x000009EF    # 2543
    li t2, 0x00625370    # 6443888
    li t3, 0x00002179    # 8569
    li t4, 0x0           # 0
    li t5, 0x00003100    # 12544

    div   a0, t0, t1     # signed   -597605 ÷ 2543
    div   a1, t2, t3     # signed    6443888 ÷ 8569
    div   a2, t4, t5     # signed         0 ÷ 12544

    # small‐number sanity: 10 ÷ 3, 10 % 3
    li   t6, 0xA        # 10
    li   s0, 0x3        # 3
    div  a3, t6, s0     # 10 ÷ 3
    rem  a4, t6, s0     # 10 % 3

    # divide by zero: 5 ÷ 0, 5 % 0
    li   t6, 0x5        # 5
    li   s0, 0x0        # 0
    div  a5, t6, s0     # 5 ÷ 0
    rem  a6, t6, s0     # 5 % 0

    nop
    nop
    ebreak
