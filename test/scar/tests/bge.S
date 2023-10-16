.global _start
_start:
# --------------------
    li x1, 5
    li x2, 0

    bge x1,x2,L1
    ebreak
L1:
    li x10, 0x00000001

# --------------------
    li x1, -4
    li x2, 5

    bge x1,x2,exit
    li x11, 0x00000002

# --------------------
    li x1, -2
    li x2, -3

    bge x1,x2,L2
    ebreak
L2:
    li x12, 0x00000003

# --------------------
    li x1, -4
    li x2, 0

    bge x1,x2,exit
    li x13, 0x00000004

# --------------------
    li x1, 0
    li x2, 0

    bge x1,x2,L3
    ebreak
L3:
    li x14, 0x00000005

# --------------------
    li x1, 0x40
    li x2, 0x100

    bge x1,x2,exit
    li x15, 0x00000006



exit:
    ebreak


ebreak
