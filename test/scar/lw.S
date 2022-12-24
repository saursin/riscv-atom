.global _start
_start:
    la x1, var1
    lw x1, 0(x1)

    la x2, var2
    lw x2, 0(x2)

    la x3, var3
    lw x3, 0(x3)

    la x4, var4
    lw x4, 0(x4)

    la x5, var5
    lw x5, 0(x5)

    la x6, var6
    lw x6, 0(x6)

    la x7, var7
    lw x7, 0(x7)

    la x8, var8
    lw x8, 0(x8)

    la x9, var9
    lw x9, 0(x9)

    la x10, var10
    lw x10, 0(x10)

    la x11, var11
    lw x11, 0(x11)

    la x12, var12
    lw x12, 0(x12)

    ebreak


.data
var1:   .word 0x12345678
var2:   .word 0x00002020
var3:   .word 0xf3232532
var4:   .word 0xdea14245
var5:   .word 0xdea12cad
var6:   .word 0xdacadaca
var7:   .word 0xbeefdead
var8:   .word 0xdeadbeef
var9:   .word 0x1beefbee
var10:   .word 0x12deadbe
var11:   .word 0x123deadb
var12:   .word 0x1234dead

# $-ASSERTIONS-$
# eq x1 0x12345678
# eq x2 0x00002020
# eq x3 0xf3232532
# eq x4 0xdea14245
# eq x5 0xdea12cad
# eq x6 0xdacadaca
# eq x7 0xbeefdead
# eq x8 0xdeadbeef
# eq x9 0x1beefbee
# eq x10 0x12deadbe
# eq x11 0x123deadb
# eq x12 0x1234dead