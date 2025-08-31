    .global _start
_start:

    li t0, 0x00091E65    # 597605
    li t1, 0x000009EF    # 2543
    li t2, 0x00625370    # 6443888
    li t3, 0x00002179    # 8569
    li t4, 0x0           # 0
    li t5, 0x00003100    # 12544

    divu a0, t0, t1      # 597605 ÷ 2543
    divu a1, t2, t3      # 6443888 ÷ 8569
    divu a2, t4, t5      # 0 ÷ 12544

    # small-number unsigned checks
    li   t6, 0xA         # 10
    li   s0, 0x3         # 3
    divu a3, t6, s0      # 10 ÷ 3
    remu a4, t6, s0      # 10 % 3

    # divide-by-zero unsigned
    li   t6, 0x5         # 5
    li   s0, 0x0         # 0
    divu a5, t6, s0      # 5 ÷ 0
    remu a6, t6, s0      # 5 % 0

    nop
    nop
    ebreak
