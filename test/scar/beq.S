.global _start
_start:
# --------------------
    li x1, 55
    li x2, 55

    beq x1,x2,L1
    ebreak
L1:
    li x10, 0x00000001

# --------------------
    li x1, 51
    li x2, -45

    beq x1,x2,exit
    li x11, 0x00000002

# --------------------
    li x1, -32
    li x2, -32

    beq x1,x2,L2
    ebreak
L2:
    li x12, 0x00000003

# --------------------
    li x1, 0
    li x2, -4

    beq x1,x2,exit
    li x13, 0x00000004

# --------------------
    li x1, 0
    li x2, 0

    beq x1,x2,L3
    ebreak
L3:
    li x14, 0x00000005

# --------------------
    li x1, 0xdeadbeef
    li x2, 0xdeafbeef

    beq x1,x2,exit
    li x15, 0x00000006



exit:
    ebreak


ebreak

# $-ASSERTIONS-$
# eq x10 0x00000001
# eq x11 0x00000002
# eq x12 0x00000003
# eq x13 0x00000004
# eq x14 0x00000005
# eq x15 0x00000006

